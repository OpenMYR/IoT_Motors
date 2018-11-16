//document references
var table = document.getElementById("body");
var command = document.getElementById("commandDisplay");
var angle = document.getElementById("angleInput");
var radioButtons = [
	document.getElementById("radio0"),
	document.getElementById("radio1"),
	document.getElementById("radio2")
];
var floatInputs = [
	document.getElementById("angleInput"),
	document.getElementById("speedAngleInput"),
];
var duration = document.getElementById("duration");
//predefines
var maxSentCommands = 10;
var buttonOpCode = ["M", "G", "S"];
var buttonCommands = ["Move", "Goto", "Stop"];
var buttonCommandDisplays = ["Move a relative distance.", "Goto an absolute position.", "Stop and dwell for some time."];
//variables
var selectedButton = -1;
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
		var queueFlag = 0;
		if (commandList.commands.length){
			queueFlag = 1;
		}
		var stepInputValue = parseInt(floatInputs[0].value);
		var stepRateInputValue = parseFloat(floatInputs[1].value);
		if(selectedButton == 2){
			addStop(queueFlag, parseFloat(duration.value));
		} else {
			addMove(buttonOpCode[selectedButton], queueFlag, stepInputValue, stepRateInputValue);
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
	radioButtons[buttonNum].style.backgroundColor = "#157e15";
	radioButtons[buttonNum].style.color = "#fff";
	radioButtons[(buttonNum + 1) % 3].style.backgroundColor = "#ffffff00";
	radioButtons[(buttonNum + 2) % 3].style.backgroundColor = "#ffffff00";
	radioButtons[(buttonNum + 1) % 3].style.color = "#000";
	radioButtons[(buttonNum + 2) % 3].style.color = "#000";
	command.innerHTML = buttonCommandDisplays[buttonNum];
	duration.style.backgroundColor = (buttonNum == 2)?"#fff":"#c3c3c3";
	duration.readOnly = !(buttonNum == 2);
	for (var a = 0; a < floatInputs.length; a++){
		floatInputs[a].style.backgroundColor = !(buttonNum == 2)?"#fff":"#c3c3c3";
		floatInputs[a].readOnly = (buttonNum == 2);
	}
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
function addStop(queueFlag,time){
	stepInputValue = Math.round(stopCountsPerSecond * time);
	addCommand("S", queueFlag, stepInputValue, stopCountsPerSecond);
}
function addMove(code, queueFlag, stepInputValue, stepRateInputValue){
	stepInputValue = Math.round(stepInputValue);
	stepRateInputValue = Math.round(stepRateInputValue);
	addCommand(code, queueFlag, stepInputValue, stepRateInputValue);
}
function addCommand(code, queueFlag, stepInputValue, stepRateInputValue){
	constructRow({cells : [code, stepInputValue, stepRateInputValue]});
	var newCommand = {
		code : code,
		data : [
			0,
			queueFlag,
			stepInputValue,
			stepRateInputValue
		]
	};
	commandList.commands.push(newCommand);
}
function clearQueue(){
	while (table.rows.length > 0)
	{
		table.deleteRow(0);
	}
	commandList.commands = [];
}