class StepperMotor {
	constructor(canvas, motorArgs, virtualMotor) {
		this.canvas = canvas;
		this._mousedown = false;
		this._touchdown = false;
		this.motorArgs = motorArgs;

		this.virtualMotor = virtualMotor;

		this._motorContext = canvas.motorCanvas.getContext("2d");
		this._angleContext = canvas.angleCanvas.getContext("2d");
		this._hornContext = canvas.hornCanvas.getContext("2d");

		StepperMotor.drawMotorBase(this._motorContext, motorArgs);
		StepperMotor.drawAngle(this._angleContext, motorArgs);
		StepperMotor.drawHorn(this._hornContext, motorArgs);

		canvas.hornCanvas.addEventListener('mousedown', event => this._mousePressed(event), false);
		canvas.hornCanvas.addEventListener('mousemove', event => this._mouseMoved(event), false);
		canvas.hornCanvas.addEventListener('mouseup', event => this._mouseUp(event), false);

		canvas.hornCanvas.addEventListener('touchstart', event => this._touchStart(event), false);
		canvas.hornCanvas.addEventListener('touchmove', event => this._touchMoved(event), false);
		canvas.hornCanvas.addEventListener('touchend', event => this._touchEnd(event), false);
	}

	_mousePressed(event) {
		StepperMotor.setMicroStepping(true);
		this._mousedown = true; 
		this._mouseMoved(event);
	}

	_mouseMoved(event) {
		if (this._mousedown) {
  			let rect = event.target.getBoundingClientRect();
			let dx = event.clientX - rect.left - rect.width/2;
			let dy = event.clientY - rect.top - rect.height/2;
			let rawAngle = Math.atan2(dx, -dy) * 180 / Math.PI;
			let delta = rawAngle - StepperMotor.mod(this.motorArgs.angle, 360);
			if (Math.abs(delta) >= 180) {
				delta += 360 * (-delta/Math.abs(delta));
			}
			this.motorArgs.angle += delta;
			this.virtualMotor.updatePosition(this.motorArgs.motorid, this.motorArgs.angle/360*200*32);
			StepperMotor.drawHorn(this._hornContext, this.motorArgs);
		}
	}

	_mouseUp(event) {
		this._mousedown = false;
	}

	_touchStart(event) {
		StepperMotor.setMicroStepping(true);
		this._touchdown = true;
		this._touchMoved(event);
	}

	_touchMoved(event) {
		if (this._touchdown) {
			let rect = event.target.getBoundingClientRect();
			let dx = event.touches[0].clientX - rect.left - rect.width/2;
			let dy = event.touches[0].clientY - rect.top - rect.height/2;
			let rawAngle = Math.atan2(dx, -dy) * 180 / Math.PI;
			let delta = rawAngle - StepperMotor.mod(this.motorArgs.angle, 360);
			if (Math.abs(delta) >= 180) {
				delta += 360 * (-delta/Math.abs(delta));
			}
			this.motorArgs.angle += delta;
			this.virtualMotor.updatePosition(this.motorArgs.motorid, this.motorArgs.angle/360*200*32);
			StepperMotor.drawHorn(this._hornContext, this.motorArgs);
		}
	}

	_touchEnd(event) {
		this._touchdown = false;
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
		for (let angle = 0; angle <= 360; angle += 5) {
			let lineLength = angle % 45 == 0 ? 15 : 10;
			ctx.save();
			ctx.rotate(angle * Math.PI / 180);
			ctx.moveTo(motor.hornLength, 0);
			ctx.lineTo(motor.hornLength + lineLength,0);
			ctx.restore();
		}
		ctx.stroke();
	}

	static mod(n, m) {
		return ((n % m) + m) % m;
	}

	static drawHorn(ctx, motor) {
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, ctx.canvas.clientWidth, ctx.canvas.clientHeight);
		ctx.save();

		ctx.translate(motor.x,motor.y);
		ctx.rotate((motor.angle-90)*Math.PI/180);

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
		ctx.moveTo(-8,-10);
		ctx.lineTo(8,-10);
		ctx.lineTo(10,-8);
		ctx.lineTo(10,8);
		ctx.lineTo(8,10);
		ctx.lineTo(-8,10);
		ctx.lineTo(-10,8);
		ctx.lineTo(-10,-8);
		ctx.lineTo(-8,-10);
		ctx.moveTo(-6.25,7);
		ctx.arc(-7, 7, .75, 0, Math.PI*2);
		ctx.moveTo(7.75,7);
		ctx.arc(7, 7, .75, 0, Math.PI*2);
		ctx.moveTo(-6.25,-7);
		ctx.arc(-7, -7, .75, 0, Math.PI*2);
		ctx.moveTo(7.75,-7);
		ctx.arc(7, -7, .75, 0, Math.PI*2);
		ctx.moveTo(5,0);
		ctx.arc(0, 0, 5, 0, Math.PI*2);
		ctx.stroke();
		ctx.restore();
	}

	static setMicroStepping(flag) {
	let out = {
		commands : [{
			code : "U",
			data : [
				motor1.motorid,
				0,
				flag ? 1 : 0,
				0
			]
		}]
	};
	let httpRequest = new XMLHttpRequest();
	httpRequest.open("POST", "/", true);
	httpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	httpRequest.send(JSON.stringify(out));
}
}