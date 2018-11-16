class ServoMotor {
	constructor(canvas, motorArgs) {
		this.canvas = canvas;
		this._mousedown = false;
		this.motorArgs = motorArgs;

		this._motorContext = canvas.motorCanvas.getContext("2d");
		this._angleContext = canvas.angleCanvas.getContext("2d");
		this._hornContext = canvas.hornCanvas.getContext("2d");

		ServoMotor.drawMotorBase(this._motorContext, motorArgs);
		ServoMotor.drawAngle(this._angleContext, motorArgs);
		ServoMotor.drawHorn(this._hornContext, motorArgs);

		canvas.hornCanvas.addEventListener('mousedown', event => this._mousePressed(event), false);
		canvas.hornCanvas.addEventListener('mousemove', event => this._mouseMoved(event), false);
		canvas.hornCanvas.addEventListener('mouseup', event => this._mouseUp(event), false);
	}

	_mousePressed(event) {
		this._mousedown = true; 
		this._mouseMoved(event);
	}

	_mouseMoved(event) {
		if (this._mousedown) {
  			let rect = event.target.getBoundingClientRect();
			let dx = event.clientX - rect.left - rect.width/2;
			let dy = event.clientY - rect.top - rect.height/2;
			let rawAngle = (Math.PI + Math.atan2(-dx, -dy)) * 180 / Math.PI;
			if ((rawAngle >= this.motorArgs.minAngle) && (rawAngle <= this.motorArgs.maxAngle)) {
				this.motorArgs.angle = rawAngle;
				ServoMotor.send(this.motorArgs.motorid-1, this.motorArgs.angle);
			}
			ServoMotor.drawHorn(this._hornContext, this.motorArgs);
		}
	}

	_mouseUp(event) {
		this._mousedown = false;
	}

	static drawAngle(ctx, motor) {
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, ctx.canvas.clientWidth, ctx.canvas.clientHeight);
		ctx.save();

		ctx.translate(motor.x, motor.y);
		ctx.rotate(-Math.PI/2);
		ctx.strokeStyle = "#000000";
		ctx.lineWidth = 1;

		ctx.beginPath();
		for (let angle = 0; angle <= 180; angle += 5) {
			let lineLength = angle % 45 == 0 ? 15 : 10;
			ctx.save();
			ctx.rotate(angle * Math.PI / 180);
			ctx.moveTo(motor.hornLength, 0);
			ctx.lineTo(motor.hornLength + lineLength,0);
			ctx.restore();
		}
		ctx.stroke();
	}

	static drawHorn(ctx, motor) {
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, ctx.canvas.clientWidth, ctx.canvas.clientHeight);
		ctx.save();

		ctx.translate(motor.x,motor.y);
		ctx.rotate((-motor.angle+90)*Math.PI/180);

		ctx.beginPath();
		ctx.strokeStyle = "#000000";
		ctx.lineWidth = 10;
		ctx.moveTo(0,0);
		ctx.lineTo(motor.hornLength - 20,0);
		ctx.stroke();
		ctx.closePath();
		ctx.beginPath();
		ctx.lineWidth = 1;
		ctx.fillStyle = '#000000';
		ctx.arc(0, 0, 8, 0, 2 * Math.PI);
		ctx.fill();
		ctx.closePath();
		ctx.beginPath();
		ctx.lineWidth = 3;
	    ctx.globalAlpha = 0.5;
		ctx.fillStyle = '#157E15';
		ctx.arc(motor.hornLength, 0, 20, 0, 2 * Math.PI);
		ctx.fill();
	    ctx.globalAlpha = 1;
		ctx.stroke();
		ctx.closePath();
		ctx.restore();
	}

	static drawMotorBase(ctx, motor) {
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, ctx.canvas.clientWidth, ctx.canvas.clientHeight);
		ctx.save();

		ctx.fillStyle = "#ffffff";
		ctx.strokeStyle = "rgba(0,0,0,255)";
		ctx.lineWidth = 0.2;

		ctx.translate(motor.x, motor.y);
		ctx.scale(motor.scale, motor.scale);

		ctx.beginPath();
		ctx.moveTo(21.775-56,20.869-30);
		ctx.lineTo(70.446-56,20.869-30);
		ctx.quadraticCurveTo(72.976-56,20.869-30,72.976-56,23.398-30);
		ctx.lineTo(72.976-56,36.546-30);
		ctx.quadraticCurveTo(72.976-56,39.075-30,70.446-56,39.075-30);
		ctx.lineTo(21.775-56,39.075-30);
		ctx.quadraticCurveTo(19.245-56,39.075-30,19.245-56,36.546-30);
		ctx.lineTo(19.245-56,23.398-30);
		ctx.quadraticCurveTo(19.245-56,20.868-30,21.775-56,20.869-30);
		ctx.moveTo(28.168-56,20.866-30);
		ctx.lineTo(64.053-56,20.866-30);
		ctx.quadraticCurveTo(65.160-56,20.866-30,65.160-56,21.973-30);
		ctx.lineTo(65.160-56,37.970-30);
		ctx.quadraticCurveTo(65.160-56,39.077-30,64.053-56,39.078-30);
		ctx.lineTo(28.168-56,39.078-30);
		ctx.quadraticCurveTo(27.060-56,39.078-30,27.060-56,37.970-30);
		ctx.lineTo(27.060-56,21.973-30);
		ctx.quadraticCurveTo(27.060-56,20.866-30,28.168-56,20.866-30);
		ctx.moveTo(25.116-56,29.972-30);
		ctx.arc(23.116-56,29.972-30,2.001,0,Math.PI*2,true);
		ctx.moveTo(71.105-56,29.972-30);
		ctx.arc(69.105-56,29.972-30,2.001,0,Math.PI*2,true);
		ctx.moveTo(56.059-56,20.883-30);
		ctx.translate(56.073-56,29.972-30);
		ctx.arc(0,0,9.08,-1.572,-2.283185307179586,1);
		ctx.translate(-9.979,0);
		ctx.arc(0,0,6.204,-1.105,-Math.PI/2,1);
		ctx.arc(0,0,6.204,-1.570,-Math.PI,1);
		ctx.arc(0,0,6.204,-3.141,-Math.PI*1.5,1); 
		ctx.arc(0,0,6.204,1.569,1.1091150706809816,1);
		ctx.translate(9.961,0);
		ctx.arc(0,0,9.089,2.484,Math.PI*1.5,1);
		ctx.translate(-56.059,-29.972);
		ctx.moveTo(59.165,29.972);	
		ctx.arc(56.059,29.972,3.106,0,Math.PI*2,true);
		ctx.moveTo(47.421,29.972);	
		ctx.arc(46.110,29.972,1.311,0,Math.PI*2,true);
		ctx.closePath();
		ctx.stroke();
		ctx.restore();
	}
	static send(motorNum, angle) {
		var command = {
			code : "G",
			data : [
				motorNum,
				0,
				Math.round(angle),
				1000
			]
		};
		var out = {
			commands : [command]
		};
		
		var httpRequest = new XMLHttpRequest();
		httpRequest.open("POST", "/", true);
		httpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
		httpRequest.send(JSON.stringify(out));
	}
}