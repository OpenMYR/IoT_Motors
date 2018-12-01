var motorCanvas = document.getElementById("motorCanvas");
var angleCanvas = document.getElementById("angleCanvas");
var hornCanvas = document.getElementById("hornCanvas");

var motorCanvas2 = document.getElementById("motorCanvas2");
var angleCanvas2 = document.getElementById("angleCanvas2");
var hornCanvas2 = document.getElementById("hornCanvas2");

var motorCanvas3 = document.getElementById("motorCanvas3");
var angleCanvas3 = document.getElementById("angleCanvas3");
var hornCanvas3 = document.getElementById("hornCanvas3");

var motorCanvas4 = document.getElementById("motorCanvas4");
var angleCanvas4 = document.getElementById("angleCanvas4");
var hornCanvas4 = document.getElementById("hornCanvas4");

let motor1 = {
	name: "Motor 1",
	motorid: 1,
	scale: 4,
	x: 150,
	y: 150,
	maxAngle: 180,
	minAngle: 0,
	angle: 90,
	hornLength: 100 
};

let motor2 = {
	name: "Motor 2",
	motorid: 2,
	scale: 4,
	x: 150,
	y: 150,
	maxAngle: 180,
	minAngle: 0,
	angle: 90,
	hornLength: 100 
};

let motor3 = {
	name: "Motor 3",
	motorid: 3,
	scale: 4,
	x: 150,
	y: 150,
	maxAngle: 180,
	minAngle: 0,
	angle: 90,
	hornLength: 100 
};

let motor4 = {
	name: "Motor 4",
	motorid: 4,
	scale: 4,
	x: 150,
	y: 150,
	maxAngle: 180,
	minAngle: 0,
	angle: 90,
	hornLength: 100 
};

let vm = new VirtualMotor({
	traverseSpeed : 500,
	transmitInterval : 50,
	motorPositions : [90, 90, 90, 90]
});

let sm = new ServoMotor({
	motorCanvas : motorCanvas,
	angleCanvas : angleCanvas,
	hornCanvas : hornCanvas,
}, motor1, vm);

let sm2 = new ServoMotor({
	motorCanvas : motorCanvas2,
	angleCanvas : angleCanvas2,
	hornCanvas : hornCanvas2,
}, motor2, vm);

let sm3 = new ServoMotor({
	motorCanvas : motorCanvas3,
	angleCanvas : angleCanvas3,
	hornCanvas : hornCanvas3,
}, motor3, vm);

let sm4 = new ServoMotor({
	motorCanvas : motorCanvas4,
	angleCanvas : angleCanvas4,
	hornCanvas : hornCanvas4,
}, motor4, vm);