#include "include/stepper_driver.h"
#include "include/waveformPulseCounter.h"
#include "Arduino.h"
#include "include/task_prio.h"

#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0

//end stop
#define GPIO_IO_A 14
#define GPIO_IO_B 12
#define MYR_DEFAULT_DEBOUNCE_MS 10 // TODO: NVS config

uint32_t direction = 0;
uint32_t paused = 0;
int32_t location = 0;
uint32_t command_done = 1;
stepper_driver *stepper_driver::instance = NULL;

uint32_t static debounceTimeMs = MYR_DEFAULT_DEBOUNCE_MS; // millis // TODO: NVS config
bool const isEndstopTrippedHigh = false;                  // Value of endstop when it is engaged.                                       // Max Home

bool static isEndstopA_ActiveNow = false;
uint32_t volatile numberOfEndstopAIsr = 0;
bool lastStateEndstopAIsr = 0;
uint32_t volatile debounceTimeoutEndstopAIsr = 0;
bool static isEndstopB_ActiveNow = false;
uint32_t volatile numberOfEndstopBIsr = 0;
bool lastStateEndstopBIsr = 0;
uint32_t volatile debounceTimeoutEndstopBIsr = 0;

static uint32_t HOMING_RATE = 6400 / 3;
static const int32_t HOMING_ENDSTOP_SEEK_STEPS = 19200;
static const uint32_t HOMING_ENDSTOP_RELEASE_STEPS = 250;
static uint32_t homing_endstop_safe_offset_steps = 0;

enum homeState
{
    notHomed = 0,
    seekingEndstop = 1,
    foundEndstop = 2,
    releaseEndstop = 3,
    moveToSoftHome = 4,
    finished = 5
};
enum homeMode
{
    noHome = 0,
    simpleQuickHome = 1,
    simpleSlowHome = 2,
    thoroughQuickHome = 3,
    thoroughSlowHome = 4
};

enum endstopMode
{
    none = 0,
    machineProtection = 1,
    homeIndicator = 2
};

//homeState homeMinState = notHomed;
//homeMode homeMinMode = simpleQuickHome;
//homeState homeMaxState = stepper_driver::notHomed;
//homeMode homeMaxMode   = stepper_driver::noHome;
static volatile homeState homeActiveState = notHomed;
static const endstopMode motorEndstopMode = endstopMode::machineProtection;

extern "C"
{
    // Called when a state change occurs during homing and new action is needed.
    // Treat as ISR as many calls are from ISRs
    void IRAM_ATTR service_home_command()
    {

        switch (homeActiveState)
        {
        default:
        case notHomed:
            // Do nothing
            break;
        case seekingEndstop: // First endstop seek complete

            homeActiveState = releaseEndstop; // Move away from endstop

            digitalWrite(GPIO_STEP_DIR, 1); // Forward
            startWaveform(10, (1 / ((float)HOMING_RATE) * 1000000) - 10, HOMING_ENDSTOP_RELEASE_STEPS);
            break;
        case releaseEndstop: // Endstop release attempt complete

            if (isEndstopA_ActiveNow)
            {
                // endstop still engaged
                homeActiveState = foundEndstop; // Move to software home position
                service_home_command();
                break;
            }

            homeActiveState = foundEndstop; // Find endstop again

            digitalWrite(GPIO_STEP_DIR, 0); // Reverse
            startWaveform(10, (1 / ((float)HOMING_RATE) * 1000000) - 10, HOMING_ENDSTOP_SEEK_STEPS);
            break;
        case foundEndstop: // Endstop found again

            homeActiveState = moveToSoftHome; // Move to software home position

            digitalWrite(GPIO_STEP_DIR, 1); // Forward
            startWaveform(10, (1 / ((float)HOMING_RATE) * 1000000) - 10, homing_endstop_safe_offset_steps);
            break;
        case moveToSoftHome:

            homeActiveState = finished; // Homing complete
            command_done = 1;
            location = 0;
            system_os_post(ACK_TASK_PRIO, 0, location);
            break;
        case finished:
            // Do nothing
            break;
        }
    }

    IRAM_ATTR void ppcb()
    {
        if (paused == 0)
        {
            if (direction == 1)
                location++;
            else
                location--;

            stepper_driver::checkLocation();
        }
    }

    IRAM_ATTR void pdcb()
    {
        if ((homeActiveState == homeState::finished) || (homeActiveState == homeState::notHomed))
        {
            command_done = 1;
            if (paused == 1)
            {
                paused = 0;
                digitalWrite(GPIO_STEP_ENABLE, 0);
            }
            system_os_post(ACK_TASK_PRIO, 0, location);
        }
        else
        {
            service_home_command();
        }
    }

    IRAM_ATTR void endstop_a_interrupt()
    {
        //stopWaveform();
        noInterrupts();
        numberOfEndstopAIsr++;
        lastStateEndstopAIsr = digitalRead(GPIO_IO_A);
        debounceTimeoutEndstopAIsr = millis();
        interrupts();
    }

    IRAM_ATTR void endstop_b_interrupt()
    {
        noInterrupts();
        numberOfEndstopBIsr++;
        lastStateEndstopBIsr = digitalRead(GPIO_IO_B);
        debounceTimeoutEndstopBIsr = millis();
        interrupts();
    }
}

stepper_driver::stepper_driver() : motor_driver()
{
    init_motor_gpio();
    setPerPulseCallback(ppcb);
    setPulsesDepletedCallback(pdcb);
    
    endstop_a_interrupt();
    endstop_b_interrupt();
    isEndstopTripped();
}

stepper_driver *IRAM_ATTR stepper_driver::getInstance()
{
    if (instance == NULL)
    {
        instance = new stepper_driver();
    }

    return instance;
}

void stepper_driver::init_motor_gpio()
{
    
    pinMode(GPIO_STEP, OUTPUT);
    pinMode(GPIO_STEP_ENABLE, OUTPUT);
    pinMode(GPIO_STEP_DIR, OUTPUT);
    pinMode(GPIO_USTEP_A, OUTPUT);
    pinMode(GPIO_IO_A, INPUT_PULLUP);
    pinMode(GPIO_IO_B, INPUT_PULLUP);

    attachInterrupt(GPIO_IO_A, endstop_a_interrupt, CHANGE);
    attachInterrupt(GPIO_IO_B, endstop_b_interrupt, CHANGE);

    setWaveformPulseCountPin(GPIO_STEP);

    digitalWrite(GPIO_USTEP_A, 0);
    digitalWrite(GPIO_STEP_ENABLE, 0);
}

#pragma region OPCODE Calls

void stepper_driver::opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    if (!((homeActiveState == homeState::finished) || (homeActiveState == homeState::notHomed)))
    {
        homeActiveState = homeState::notHomed;
    }
    //Serial.printf("move %d steps at %d steps per sec", step_num, step_rate);
    if (isEndstopTripped())
        return;

    stopWaveform();
    
    uint32_t limit = abs(step_num);
    direction = step_num > 0 ? 1 : 0;
    digitalWrite(GPIO_STEP_DIR, direction);
    if (limit > 0)
    {
        command_done = 0;
        startWaveform(10, (1 / ((float)step_rate) * 1000000) - 10, limit);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}

void stepper_driver::opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    if (!((homeActiveState == homeState::finished) || (homeActiveState == homeState::notHomed)))
    {
        homeActiveState = homeState::notHomed;
    }
    if (isEndstopTripped())
        return;
        
    stopWaveform();
    //Serial.printf("goto %d %d\n", step_num, step_rate);
    
    uint32_t limit = 0;

    if(location > step_num)
    {
        limit = (location - step_num);
        direction = 0;
    }
    else if(location < step_num)
    {
        limit = (step_num - location);
        direction = 1;
    }
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveform(10, (1 / ((float)step_rate) * 1000000) - 10, limit);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}

void stepper_driver::opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id)
{
    if (!((homeActiveState == homeState::finished) || (homeActiveState == homeState::notHomed)))
    {
        homeActiveState = homeState::notHomed;
    }

    if (isEndstopTripped())
        return;
        
    stopWaveform();
    //Serial.printf("stop %d %d\n", wait_time, precision);
    uint32_t limit = abs(wait_time);
    if(limit == 0)
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
    else
    {
        command_done = 0;
        paused = 1;
        digitalWrite(GPIO_STEP_ENABLE, 1);
        startWaveform(10, (1 / ((float)precision) * 1000000) - 10, limit);
    }
}

// @param step_num
// @param step_rate
void stepper_driver::opcode_home(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    //signed int p1 = -HOMING_ENDSTOP_SEEK_STEPS; //step_num;
    //unsigned short p2 = HOMING_RATE; //step_rate;

    //if(endstop_a || endstop_b)
    //    return;

    stopWaveform();
    command_done = 0;
    digitalWrite(GPIO_USTEP_A, 1);

    // Move and engage endstop
    homeActiveState = homeState::seekingEndstop;
    digitalWrite(GPIO_STEP_DIR, 0); // Reverse
    startWaveform(10, (1 / ((float)HOMING_RATE) * 1000000) - 10, HOMING_ENDSTOP_SEEK_STEPS);
    
}

#pragma endregion

bool stepper_driver::is_motor_running(uint8_t motor_id)
{
    return command_done == 0;
}

void stepper_driver::driver_logic_task(os_event_t *events)
{

}

void stepper_driver::driver()
{

}

float stepper_driver::calculate_step_incrementor(unsigned short input_step_rate)
{
    return 0;
}

void stepper_driver::change_motor_setting(config_setting setting, uint32_t data1, uint32_t data2)
{
    if (setting == config_setting::MICROSTEPPING)
    {
        digitalWrite(GPIO_USTEP_A, data1 > 0);
    }
}

void IRAM_ATTR stepper_driver::checkLocation()
{
    if (isEndstopTripped(true))
    {
        if (isEndstopTripped() && ((homeActiveState == homeState::finished) || (homeActiveState == homeState::notHomed)))
        {
            stopWaveform();
            command_done = true;
            system_os_post(ESTOP_TASK_PRIO, 0, 0);
        }
        else
        {
            if (isEndstopA_ActiveNow && (homeActiveState == homeState::seekingEndstop || homeActiveState == homeState::foundEndstop))
            {
                service_home_command();
            }
/*             else
            {
                stopWaveform();
                command_done = true;
                system_os_post(ESTOP_TASK_PRIO, 0, 0);
            } */
        }
    }
}

bool IRAM_ATTR stepper_driver::isEndstopTripped(bool rawState)
{
    uint32_t saveDebounceTimeout;
    bool saveLastState;
    uint32_t hasChanged;
    bool currentState = false;
    bool retunValue = false;

    //endstopA
    noInterrupts();
    hasChanged = numberOfEndstopAIsr;
    saveDebounceTimeout = debounceTimeoutEndstopAIsr;
    saveLastState = lastStateEndstopAIsr;
    interrupts();

    currentState = digitalRead(GPIO_IO_A);

    // if Interrupt Has triggered AND pin is in same state AND the debounce time has expired THEN endstop is stable
    if ((hasChanged != 0) && (currentState == saveLastState) && (millis() - saveDebounceTimeout > debounceTimeMs))
    {
        noInterrupts();
        numberOfEndstopAIsr = 0; // clear counter
        interrupts();

        if (currentState == isEndstopTrippedHigh)
        {
            isEndstopA_ActiveNow = true;
        }
        else
        {
            isEndstopA_ActiveNow = false;
        }
    }

    retunValue |= isEndstopA_ActiveNow;

    //endstopB
    noInterrupts();
    hasChanged = numberOfEndstopBIsr;
    saveDebounceTimeout = debounceTimeoutEndstopBIsr;
    saveLastState = lastStateEndstopBIsr;
    interrupts();

    currentState = digitalRead(GPIO_IO_B);

    // if Interrupt Has triggered AND pin is in same state AND the debounce time has expired THEN endstop is stable
    if ((hasChanged != 0) && (currentState == saveLastState) && (millis() - saveDebounceTimeout > debounceTimeMs))
    {
        noInterrupts();
        numberOfEndstopBIsr = 0; // clear counter
        interrupts();

        if (currentState == isEndstopTrippedHigh)
        {
            isEndstopB_ActiveNow = true;
        }
        else
        {
            isEndstopB_ActiveNow = false;
        }
    }

    retunValue |= isEndstopB_ActiveNow;

    if(!rawState && ((motorEndstopMode == endstopMode::none) || (motorEndstopMode == endstopMode::homeIndicator)))
    {
        retunValue = 0;
    }

    return retunValue;
}

bool IRAM_ATTR stepper_driver::isEndstopTripped()
{
    isEndstopTripped(false);
}

uint32_t stepper_driver::set_machine_software_microSteps_offset(uint32_t offset)
{
    homing_endstop_safe_offset_steps = offset;
    return homing_endstop_safe_offset_steps;
}
