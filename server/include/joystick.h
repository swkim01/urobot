#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define JOYSTICK_DESCRIPTION \
"var Joystick	= function(opts)\n" \
"{\n" \
"    opts = opts || {};\n" \
"    this._container = opts.container || document.body;\n" \
"    this._strokeStyle = opts.strokeStyle || 'cyan';\n" \
"    this._lineStyle = opts.lineStyle || 'red';\n" \
"\n" \
"    this._container.style.position = \"relative\";\n" \
"    this._width   = this._container.offsetWidth;\n" \
"    this._height  = this._container.offsetHeight;\n" \
"    this._offsetX = this._container.offsetLeft;\n" \
"    this._offsetY = this._container.offsetTop;\n" \
"    this._centerX = this._width / 2;\n" \
"    this._centerY = this._height / 2;\n" \
"    this._radius  = (this._width < this._height) ? this._width / 2 : this._height / 2;\n" \
"    this._movementRadius = this._radius / 2;\n" \
"    this._movementRange	= 10;\n" \
"    this._stickEl = opts.stickElement || this._buildJoystickStick();\n" \
"    this._baseEl  = opts.baseElement  || this._buildJoystickBase();\n" \
"    this._lineEl  = opts.lineElement  || this._buildJoystickLine();\n" \
"    this._mouseSupport	= opts.mouseSupport !== undefined ? opts.mouseSupport : false;\n" \
"\n" \
"    this._container.appendChild(this._baseEl);\n" \
"    this._baseEl.style.position	= \"absolute\";\n" \
"    this._baseEl.style.display	= \"\";\n" \
"    this._baseEl.style.left = (this._centerX-this._baseEl.width /2)+\"px\";\n" \
"    this._baseEl.style.top  = (this._centerY-this._baseEl.height /2)+\"px\";\n" \
"\n" \
"    this._container.appendChild(this._stickEl);\n" \
"    this._stickEl.style.position = \"absolute\";\n" \
"    this._stickEl.style.display	= \"\";\n" \
"    this._stickEl.style.left = (this._centerX-this._stickEl.width /2)+\"px\";\n" \
"    this._stickEl.style.top  = (this._centerY-this._stickEl.height /2)+\"px\";\n" \
"\n" \
"    this._container.appendChild(this._lineEl);\n" \
"    this._lineEl.style.position	= \"absolute\";\n" \
"    this._lineEl.style.display	= \"none\";\n" \
"    \n" \
"    this._pressed	= false;\n" \
"    this._touchIdx	= null;\n" \
"    this._baseX	= this._centerX;\n" \
"    this._baseY	= this._centerY;\n" \
"    this._stickX	= this._centerX;\n" \
"    this._stickY	= this._centerY;\n" \
"    this._numberOfFrames = 5;\n" \
"    this._count	= 0;\n" \
"\n" \
"    var __bind	= function(fn, me){ return function(){ return fn.apply(me, arguments); }; };\n" \
"    this._$onTouchStart	= __bind(this._onTouchStart , this);\n" \
"    this._$onTouchEnd	= __bind(this._onTouchEnd   , this);\n" \
"    this._$onTouchMove	= __bind(this._onTouchMove  , this);\n" \
"    this._container.addEventListener('touchstart', this._$onTouchStart, false);\n" \
"    this._container.addEventListener('touchend'  , this._$onTouchEnd,   false);\n" \
"    this._container.addEventListener('touchmove' , this._$onTouchMove,  false);\n" \
"    if( this._mouseSupport ){\n" \
"       this._$onMouseDown = __bind(this._onMouseDown	, this);\n" \
"       this._$onMouseUp	  = __bind(this._onMouseUp	, this);\n" \
"       this._$onMouseMove = __bind(this._onMouseMove	, this);\n" \
"       this._container.addEventListener('mousedown', this._$onMouseDown, false);\n" \
"       this._container.addEventListener('mouseup'  , this._$onMouseUp,   false);\n" \
"       this._container.addEventListener('mousemove', this._$onMouseMove, false);\n" \
"    }\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.destroy	= function()\n" \
"{\n" \
"    this._container.removeChild(this._baseEl);\n" \
"    this._container.removeChild(this._stickEl);\n" \
"    this._container.removeChild(this._lineEl);\n" \
"    \n" \
"    this._container.removeEventListener('touchstart', this._$onTouchStart, false);\n" \
"    this._container.removeEventListener('touchend' , this._$onTouchEnd, false);\n" \
"    this._container.removeEventListener('touchmove', this._$onTouchMove, false);\n" \
"    if( this._mouseSupport ){\n" \
"       this._container.removeEventListener('mouseup', this._$onMouseUp, false);\n" \
"       this._container.removeEventListener('mousedown', this._$onMouseDown, false);\n" \
"       this._container.removeEventListener('mousemove', this._$onMouseMove, false);\n" \
"    }\n" \
"}\n" \
"\n\n" \
"Joystick.touchScreenAvailable	= function()\n" \
"{\n" \
"    return 'createTouch' in document ? true : false;\n" \
"}\n" \
"\n\n" \
";(function(destObj){\n" \
"    destObj.addEventListener	= function(event, fct){\n" \
"       if(this._events === undefined) 	this._events	= {};\n" \
"       this._events[event] = this._events[event]	|| [];\n" \
"       this._events[event].push(fct);\n" \
"       return fct;\n" \
"    };\n" \
"    destObj.removeEventListener	= function(event, fct){\n" \
"       if(this._events === undefined) 	this._events	= {};\n" \
"       if( event in this._events === false  )	return;\n" \
"       this._events[event].splice(this._events[event].indexOf(fct), 1);\n" \
"    };\n" \
"    destObj.dispatchEvent = function(event){\n" \
"       if(this._events === undefined) 	this._events	= {};\n" \
"       if( this._events[event] === undefined )	return;\n" \
"       var tmpArray	= this._events[event].slice();\n" \
"       for(var i = 0; i < tmpArray.length; i++){\n" \
"          var result = tmpArray[i].apply(this, Array.prototype.slice.call(arguments, 1));\n" \
"          if( result !== undefined )	return result;\n" \
"       }\n" \
"       return undefined;\n" \
"    };\n" \
"})(Joystick.prototype);\n" \
"\n\n" \
"Joystick.prototype.deltaX = function(){ return this._stickX - this._baseX; }\n" \
"\n\n" \
"Joystick.prototype.deltaY = function(){ return this._stickY - this._baseY; }\n" \
"\n\n" \
"Joystick.prototype.up	= function(){\n" \
"    if( this._pressed === false ) return false;\n" \
"    var deltaX	= this.deltaX();\n" \
"    var deltaY	= this.deltaY();\n" \
"    if( deltaY >= 0 ) return false;\n" \
"    if( Math.abs(deltaX) > 2*Math.abs(deltaY) )	return false;\n" \
"    return true;\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.down	= function(){\n" \
"    if( this._pressed === false ) return false;\n" \
"    var deltaX	= this.deltaX();\n" \
"    var deltaY	= this.deltaY();\n" \
"    if( deltaY <= 0 ) return false;\n" \
"    if( Math.abs(deltaX) > 2*Math.abs(deltaY) )	return false;\n" \
"    return true;	\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.right = function(){\n" \
"    if( this._pressed === false ) return false;\n" \
"    var deltaX	= this.deltaX();\n" \
"    var deltaY	= this.deltaY();\n" \
"    if( deltaX <= 0 ) return false;\n" \
"    if( Math.abs(deltaY) > 2*Math.abs(deltaX) )	return false;\n" \
"    return true;	\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.left	= function(){\n" \
"    if( this._pressed === false ) return false;\n" \
"    var deltaX	= this.deltaX();\n" \
"    var deltaY	= this.deltaY();\n" \
"    if( deltaX >= 0 ) return false;\n" \
"    if( Math.abs(deltaY) > 2*Math.abs(deltaX) )	return false;\n" \
"    return true;	\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onUp = function()\n" \
"{\n" \
"    this._count = this._numberOfFrames;\n" \
"    this._lineEl.style.display	= \"none\";\n" \
"    for (var i = 0; i < this._numberOfFrames; i++) {\n" \
"        setTimeout(function(){returnToCenter();}, i*100);\n" \
"    }\n" \
"    //setTimeout(function(){returnToCenter();}, 100);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onDown = function(x, y)\n" \
"{\n" \
"    this._pressed	= true;\n" \
"    this._stickX	= x;\n" \
"    this._stickY	= y;\n" \
"    this.constrainCircle();\n" \
"    this._stickEl.style.left = (this._stickX - this._stickEl.width /2)+\"px\";\n" \
"    this._stickEl.style.top  = (this._stickY - this._stickEl.height/2)+\"px\";\n" \
"    this._lineEl.style.display = \"\";\n" \
"    this.drawLine();\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onMove = function(x, y)\n" \
"{\n" \
"    if( this._pressed === true ){\n" \
"       this._stickX	= x;\n" \
"       this._stickY	= y;\n" \
"       this.constrainCircle();\n" \
"       this._stickEl.style.left = (this._stickX - this._stickEl.width /2)+\"px\";\n" \
"       this._stickEl.style.top  = (this._stickY - this._stickEl.height/2)+\"px\";\n" \
"       this.drawLine();\n" \
"       this.sendMoveCommand(true);\n" \
"    }\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.drawLine = function() {\n" \
"    var ctx		= this._lineEl.getContext('2d');\n" \
"    ctx.clearRect(0,0,this._width, this._height);\n" \
"    ctx.beginPath();\n" \
"    ctx.strokeStyle	= this._lineStyle;\n" \
"    ctx.lineWidth	= 4;\n" \
"    /*ctx.moveTo(this._width/2, this._height/2);*/\n" \
"    ctx.moveTo(this._lineEl.width/2, this._lineEl.height/2);\n" \
"    ctx.lineTo(this._stickX, this._stickY);\n" \
"    ctx.stroke();\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.constrainCircle = function() {\n" \
"    var diffX = this.deltaX();\n" \
"    var diffY = this.deltaY();\n" \
"    var radial = Math.sqrt((diffX*diffX) + (diffY*diffY));\n" \
"    if ( radial > this._movementRadius ) {\n" \
"       this._stickX = this._baseX + (diffX / radial) * this._movementRadius;\n" \
"       this._stickY = this._baseY + (diffY / radial) * this._movementRadius;\n" \
"    }\n" \
"}\n" \
"\n\n" \
"Joystick.prototype.sendMoveCommand = function(enableAck) {\n" \
"    var cartX = this.deltaX() / this._movementRadius * this._movementRange;\n" \
"    var cartY = this.deltaY() / this._movementRadius * this._movementRange;\n" \
"    var radial = parseInt(Math.sqrt((cartX*cartX) + (cartY*cartY)));\n" \
"    var angle = Math.atan2(cartY, cartX);\n" \
"    motion(\"Move\"+\"&args=\"+radial+\",\"+angle, enableAck);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onMouseUp	= function(event)\n" \
"{\n" \
"    return this._onUp();\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onMouseDown	= function(event)\n" \
"{\n" \
"    var x = event.clientX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = event.clientY - this._offsetY + document.body.scrollTop;\n" \
"    return this._onDown(x, y);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onMouseMove	= function(event)\n" \
"{\n" \
"    var x = event.clientX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = event.clientY - this._offsetY + document.body.scrollTop;\n" \
"    return this._onMove(x, y);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onTouchStart = function(event)\n" \
"{\n" \
"    if( this._touchIdx !== null ) return;\n" \
"    \n" \
"    var isValid	= this.dispatchEvent('touchStartValidation', event);\n" \
"    if( isValid === false ) return;\n" \
"    \n" \
"    event.preventDefault();\n" \
"    var touch = event.changedTouches[0];\n" \
"    this._touchIdx = touch.identifier;\n" \
"    \n" \
"    var x = touch.clientX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = touch.clientY - this._offsetY + document.body.scrollTop;\n" \
"    return this._onDown(x, y);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onTouchEnd	= function(event)\n" \
"{\n" \
"    if( this._touchIdx === null )	return;\n" \
"    \n" \
"    var touchList = event.changedTouches;\n" \
"    for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++);\n" \
"    if( i === touchList.length)	return;\n" \
"    \n" \
"    this._touchIdx = null;\n" \
"    \n" \
"    event.preventDefault();\n" \
"    \n" \
"    return this._onUp();\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._onTouchMove	= function(event)\n" \
"{\n" \
"    if( this._touchIdx === null )	return;\n" \
"    \n" \
"    var touchList	= event.changedTouches;\n" \
"    for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++ );\n" \
"    if( i === touchList.length)	return;\n" \
"    var touch	= touchList[i];\n" \
"    \n" \
"    event.preventDefault();\n" \
"    \n" \
"    var x = touch.clientX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = touch.clientY - this._offsetY + document.body.scrollTop;\n" \
"    return this._onMove(x, y);\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._buildJoystickBase	= function()\n" \
"{\n" \
"    var canvas	= document.createElement( 'canvas' );\n" \
"    canvas.width	= this._radius;\n" \
"    canvas.height	= this._radius;\n" \
"    var ctx		= canvas.getContext('2d');\n" \
"    ctx.beginPath();\n" \
"    ctx.strokeStyle	= this._strokeStyle;\n" \
"    ctx.lineWidth	= 2;\n" \
"    ctx.arc( canvas.width/2, canvas.width/2, canvas.width/2 - ctx.lineWidth, 0, Math.PI*2, true);\n" \
"    ctx.stroke();\n" \
"    return canvas;\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._buildJoystickStick	= function()\n" \
"{\n" \
"    var canvas	= document.createElement( 'canvas' );\n" \
"    canvas.width	= this._radius / 2;\n" \
"    canvas.height	= this._radius / 2;\n" \
"    var ctx		= canvas.getContext('2d');\n" \
"    ctx.beginPath();\n" \
"    ctx.fillStyle	= this._strokeStyle;\n" \
"    ctx.lineWidth	= 2;\n" \
"    ctx.arc( canvas.width/2, canvas.width/2, canvas.width/2 - ctx.lineWidth, 0, Math.PI*2, true);\n" \
"    ctx.fill();\n" \
"    return canvas;\n" \
"}\n" \
"\n\n" \
"Joystick.prototype._buildJoystickLine	= function()\n" \
"{\n" \
"    var canvas	= document.createElement( 'canvas' );\n" \
"    canvas.width	= this._width;\n" \
"    canvas.height	= this._height;\n" \
"    return canvas;\n" \
"}\n" \
"\n\n" \
"function returnToCenter() {\n" \
"    joystick._stickX -= (joystick._stickX - joystick._baseX) / joystick._count;\n" \
"    joystick._stickY -= (joystick._stickY - joystick._baseY) / joystick._count;\n" \
"    joystick._count--;\n" \
"    \n" \
"    if (joystick._count == 0) {\n" \
"       joystick._pressed	= false;\n" \
"       joystick._stickX	= joystick._baseX;\n" \
"       joystick._stickY	= joystick._baseY;\n" \
"    }\n" \
"    joystick._stickEl.style.left = (joystick._stickX - joystick._stickEl.width /2)+\"px\";\n" \
"    joystick._stickEl.style.top  = (joystick._stickY - joystick._stickEl.height/2)+\"px\";\n" \
"    joystick.sendMoveCommand(false);\n" \
"    if (joystick._count != 0) \n" \
"        setTimeout(function(){returnToCenter();}, 100);\n" \
"}\n"
/*
"    var x = event.clientX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = event.clientY - this._offsetY + document.body.scrollTop;\n" \
*/
/*
"    var x = touch.pageX - this._offsetX + document.body.scrollLeft;\n" \
"    var y = touch.pageY - this._offsetY + document.body.scrollTop;\n" \
*/

#define JOYSTICK_DESCRIPTION_LEN strlen (JOYSTICK_DESCRIPTION)

#define JOYSTICK_LOCATION "/web/joystick.js"
#define JOYSTICK_PAGE_CONTENT_TYPE "text/javascript"

#endif /* JOYSTICK_H_ */
