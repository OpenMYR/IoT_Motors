var httpRequest = new XMLHttpRequest();

document.getElementById("connect").addEventListener("click", function(){
	var parameters={
		code : "C",
		data : [
			document.getElementById("ssid").value,
			document.getElementById("pass").value
		]
	};
	if (parameters.ssid != ""){
		send(parameters);
	} else {
		alert("\"Network Name\" cannot be left blank");
	}
});
document.getElementById("disconnect").addEventListener("click", function(){
	var parameters={
		code : "D",
		data : [
			"",
			""
		]
	};
	send(parameters);
});

document.getElementById("change").addEventListener("click", function(){
	if (document.getElementById("newOTAPass1").value !== document.getElementById("newOTAPass2").value) {
		alert("Confrimation password does not match");
	} else {
		var parameters={
			code : "O",
			data : [
				"",
				""
			]
		};
		send(parameters);
	}
});
function send (args){
	let out = {
		commands : [args]
	};
	httpRequest.open("POST", "/", true);
	httpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	httpRequest.send(JSON.stringify(out));
}