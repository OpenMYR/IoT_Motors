class VirtualMotor {
	constructor(args) {
		this.traverseSpeed = args.traverseSpeed;
		this.transmitInterval = args.transmitInterval;
		this.motorPositions = args.motorPositions.slice();
		this.oldMotorPositions = args.motorPositions.slice();
		this.changed = false;
		this.send();
		this.launchLoop();
	}

	launchLoop() {
        (function(scope){
			scope.loop = setInterval(function(){
				scope.tryUpdate();
			}, scope.transmitInterval);
		})(this);
	}

	updatePosition(motorid, position) {
		this.motorPositions[motorid] = position;
		this.changed = true;
		this.tryUpdate();
	}

	tryUpdate() {
		if (this.changed) {
			if (Date.now() >= this.lastTransmition + this.transmitInterval) {
				this.send();
				this.changed = false;
			}
		}
	}

	send() {
		let commands = [];
		for (let i = 0; i < this.motorPositions.length; i++) {
			if(this.motorPositions[i] != this.oldMotorPositions[i]) {
				commands.push({
					code : "G",
					data : [
						i,
						0,
						Math.round(this.motorPositions[i]),
						this.traverseSpeed
					]
				});
				this.oldMotorPositions[i] = this.motorPositions[i];
			}
		}
		let out = {
			commands : commands
		};
		
		if (commands.length > 0) {
			let httpRequest = new XMLHttpRequest();
			httpRequest.open("POST", "/", true);
			httpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
			httpRequest.send(JSON.stringify(out));
		}
		this.lastTransmition = Date.now();
	}
}