//document references
var table = document.getElementById("body");
var command = document.getElementById("commandDisplay");
var angle = document.getElementById("angleInput");
var commandButtons = [
	document.getElementById("cr0"),
	document.getElementById("cr1"),
	document.getElementById("cr2")
];
var unitButtons = [
	document.getElementById("unitDegree"),
	document.getElementById("unitStep")
];
var unitLabels = [
	document.getElementById("amount"),
	document.getElementById("speed")
];
var floatInputs = [
	document.getElementById("angleInput"),
	document.getElementById("speedAngleInput"),
];
var duration = document.getElementById("duration");
var microSteppingCheckbox = document.getElementById("microStepCheckbox");
//Motor constants
var stepsPerRotation = 200;
var microSteps = 32;
//predefines
var maxSentCommands = 10;
var buttonOpCode = ["M", "G", "S"];
var buttonCommands = ["Move", "Goto", "Stop"];
var amountUnits = ["Degrees", "Steps"];
var speedUnits = ["Degrees/Sec", "Steps/Sec"];
var c = 261, d = 294, e = 329, f = 349, g = 391, gS = 415, a = 440, aS = 455, b = 466, cH = 523,
	cSH = 554, dH = 587, dSH = 622, eH = 659, fH = 698, fSH = 740, gH = 784, gSH = 830, aH = 880;
var buttonCommandDisplays = ["Move a relative distance.", "Goto an absolute position.", "Stop and dwell for some time."];
//variables
var selectedButton = -1;
var selectedUnit = 0;
var isMicroStepping = false;
var lastMicroStepping = -1;
var moreDataWaiting = false;
var stopCountsPerSecond = 1000;
var dataParts = [];
var httpRequest = new XMLHttpRequest();
httpRequest.onreadystatechange=function(){
	if (httpRequest.readyState==4){
		if (httpRequest.status==202){
			if (dataParts.length > 0){
				send(dataParts.shift());
			}
		}
		else{
			dataParts = [];
			alert("An error has occured making the request");
		}
	}
}
var commandList = {
	commands : []
};
//register events
for (var i = 0; i < floatInputs.length; i++) {
	floatInputs[i].addEventListener("blur", bluredFloatInput);
	floatInputs[i].addEventListener("keypress", changedFloatInput);
}
duration.addEventListener("blur", bluredTimeInput);
document.getElementById("add").addEventListener("click", function(){
	if (isValidCommand()){
		isMicroStepping = microStepCheckbox.checked;
		if(selectedButton == 2){
			addStop(0, parseFloat(duration.value));
		} else {
			addMove(0, buttonOpCode[selectedButton], getSteps(), getStepRate());
		}
	}
});
document.getElementById("execute").addEventListener("click", function(){
	send(commandList);
});
function constructRow(args) {
	var row = table.insertRow(table.rows.length);
	row.insertCell().innerHTML = "<b>" + args.cells[0] + "</b>";
	row.insertCell().innerHTML = args.cells[1];
	row.insertCell().innerHTML = args.cells[2];
	return row;
}
function modeSelected(buttonNum){
	selectedButton = buttonNum;
	commandButtons[buttonNum].style.backgroundColor = "#157e15";
	commandButtons[buttonNum].style.color = "#fff";
	commandButtons[(buttonNum + 1) % 3].style.backgroundColor = "#ffffff00";
	commandButtons[(buttonNum + 2) % 3].style.backgroundColor = "#ffffff00";
	commandButtons[(buttonNum + 1) % 3].style.color = "#000";
	commandButtons[(buttonNum + 2) % 3].style.color = "#000";
	command.innerHTML = buttonCommandDisplays[buttonNum];
	duration.style.backgroundColor = (buttonNum == 2)?"#fff":"#c3c3c3";
	duration.readOnly = !(buttonNum == 2);
	for (var a = 0; a < floatInputs.length; a++){
		floatInputs[a].style.backgroundColor = !(buttonNum == 2)?"#fff":"#c3c3c3";
		floatInputs[a].readOnly = (buttonNum == 2);
	}
}
function unitSelected(unitNum){
	selectedUnit = unitNum;
	unitButtons[1 - unitNum].style.backgroundColor = "#ffffff00";
	unitButtons[1 - unitNum].style.color = "#000";
	unitButtons[unitNum].style.backgroundColor = "#157e15";
	unitButtons[unitNum].style.color = "#fff";
	unitLabels[0].innerHTML = amountUnits[unitNum];
	unitLabels[1].innerHTML = speedUnits[unitNum];
}
function changedFloatInput(event){
	//enter or -
	if ((event.charCode == 13) || (event.charCode == 45)){
		return true;
	}
	// "." if one is not already there
	if (event.charCode == 46 && (event.target.value.indexOf('.') == -1)){
		return true;
	}
	//delete or backspace
	if (event.charCode == 0){
		return true;
	}
	//numbers
	if (!(event.charCode >= 48 && event.charCode <= 57)){
		if (event.preventDefault) {
			event.preventDefault();
		} else {
			event.returnValue = false;
		}
	}
}
function bluredFloatInput(event){
	var valueFloat= parseFloat(event.target.value || 0);
	if (!isNaN(valueFloat)){
		event.target.value = valueFloat;
	} else {
		event.target.value = 0;
	}
}
function bluredTimeInput(event){
	var valueFloat= parseFloat(event.target.value || 0);
	if (!isNaN(valueFloat)){
		event.target.value = valueFloat + " Seconds";
	} else {
		event.target.value = 0 + " Seconds";
	}
}
function send(args){
	if (args.commands.length > maxSentCommands) {
		dataParts = [];
		for (var i = 0; i < args.commands.length; i += maxSentCommands){
			var newCommandList = {
				commands : args.commands.slice(i, Math.min(i + maxSentCommands, args.commands.length))
			};
			dataParts.push(newCommandList);
		}
		args = dataParts.shift();
	}
	httpRequest.open("POST", "/", true);
	httpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	httpRequest.send(JSON.stringify(args));
}
function getSteps() {
	var num = parseInt(floatInputs[0].value);
	if (selectedUnit == 1) return num;
	return (num / 360) * stepsPerRotation * (isMicroStepping ? microSteps : 1);
}
function getStepRate() {
	var num = parseInt(floatInputs[1].value);
	if (selectedUnit == 1) return num;
	return (num / 360) * stepsPerRotation * (isMicroStepping ? microSteps : 1);
}
function isValidCommand(){
	isValid = true;
	errMessage = ""; 
	if (selectedButton == -1) {
		errMessage += "Please select a command.\n";
		isValid = false;
	}
	if (Math.round(parseFloat(floatInputs[1].value)) < 0) {
		errMessage += "Speed must be positive.\n";
		isValid = false;
	}
	if ((Math.round(parseFloat(duration.value)) < 0) && (selectedButton == 2)) {
		errMessage += "Stop time must be positive.\n";
		isValid = false;
	}
	if (!isValid){
		alert(errMessage);
	}
	return isValid;
}
function imperialMarch(){
	lastMicroStepping = -1;
	isMicroStepping = false;
	var song = [a, .5, a, .5,a, .5,f, .35 , cH, .15 , a, .5 , f,
		.35 , cH, .15 , a, .65, "S" , .15, eH, .5 , eH, .5 , eH,
		.5 , fH, .35 , cH, .15 , gS, .5 , 	f, .35 , cH, .15 , a,
		.65];
	clearQueue();
	playSong(song);
}
function playSong(song){
	addStop(0,0,0);
	for (var a = 0; a < song.length; a+=2){
		if (song[a] == "S"){
			addStop(0, song[a+1]);
		} else {
			addMove(0, "M", 10, 1500);
			addMove(0, "M", song[a] * song[a+1] * 4, song[a] * 4);
		}
		addStop(0,.01,1000);
	}
}
function checkMicroStep(motorid){
	if (lastMicroStepping != isMicroStepping) {
		var queueFlag = commandList.commands.length ? 1 : 0;
		lastMicroStepping = isMicroStepping;
		addCommand(motorid, "U", queueFlag, isMicroStepping ? 1 : 0, 0);
	}
}
function addStop(motorid, time){
	checkMicroStep(0);
	var queueFlag = commandList.commands.length ? 1 : 0;
	stepInputValue = Math.round(stopCountsPerSecond * time);
	addCommand(motorid, "S", queueFlag, stepInputValue, stopCountsPerSecond);
}
function addMove(motorid, code, stepInputValue, stepRateInputValue){
	checkMicroStep(0);
	var queueFlag = commandList.commands.length ? 1 : 0;
	stepInputValue = Math.round(stepInputValue);
	stepRateInputValue = Math.round(stepRateInputValue);
	addCommand(motorid, code, queueFlag, stepInputValue, stepRateInputValue);
}
function addCommand(motorid, code, queueFlag, stepInputValue, stepRateInputValue){
	constructRow({cells : [code, stepInputValue, stepRateInputValue]});
	var newCommand = {
		code : code,
		data : [
			motorid,
			queueFlag,
			stepInputValue,
			stepRateInputValue
		]
	};
	commandList.commands.push(newCommand);
}
function clearQueue(){
	lastMicroStepping = -1;
	while (table.rows.length > 0)
	{
		table.deleteRow(0);
	}
	commandList.commands = [];
}