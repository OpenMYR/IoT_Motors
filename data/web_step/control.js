var motorCanvas = document.getElementById("motorCanvas");
var angleCanvas = document.getElementById("angleCanvas");
var hornCanvas = document.getElementById("hornCanvas");

let motor1 = {
	name: "Motor 0",
	motorid: 0,
	scale: 12,
	x: 250,
	y: 250,
	angle: 0,
	hornLength: 160 
};

let vm = new VirtualMotor({
		traverseSpeed : 12800,
		transmitInterval : 100,
		motorPositions : [0]
});

let sm = new StepperMotor({
	motorCanvas : motorCanvas,
	angleCanvas : angleCanvas,
	hornCanvas : hornCanvas,
}, motor1, vm);