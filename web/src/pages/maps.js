import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'

import './maps.css';

class maps extends Component {
	state = {
		mapSize: [1000, 1000],
		mapMouseMove: [0, 0],
	};

	canvas = null;
	canvasContext = null;
	images = {};

	mapMouseDownPos = null;

	mapScale = 1;
	mapPosition = [0, 0];

	getblock = (x, y) => {
		if (x % 10 === 0)
			return this.images.diamond_ore;
		else if (y % 10 === 0)
			return this.images.emerald_ore;
		else
			return this.images.stone;
	}

	draw = () => {
		if (this.canvas) {
			this.canvasContext.clearRect(0, 0, this.canvas.width, this.canvas.height)
			if (this.mapScale > 0.5) {
				for (let x = Math.round(- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x <= Math.round(this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x++)for (let y = Math.round(- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y <= Math.round(this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y++) {
					this.canvasContext.drawImage(
						this.getblock(x, y),
						0, 0, 16, 16,
						this.canvas.width / 2 + (x - 0.5 + this.mapPosition[0]) * 16 * this.mapScale,
						this.canvas.height / 2 + (y - 0.5 + this.mapPosition[1]) * 16 * this.mapScale,
						16 * this.mapScale,
						16 * this.mapScale);
				}
			}
		}
	}

	componentDidMount() {
		(this.images.diamond_ore = new Image()).src = diamond_ore;
		(this.images.emerald_ore = new Image()).src = emerald_ore;
		(this.images.stone = new Image()).src = stone;
	}

	handleMouseDown = event => {
		event.preventDefault();
		this.mapMouseDownPos = [event.clientX, event.clientY];
	}
	handleMouseMove = event => {
		if (this.mapMouseDownPos) {
			event.preventDefault();
			this.setState({
				mapMouseMove: [
					event.clientX - this.mapMouseDownPos[0],
					event.clientY - this.mapMouseDownPos[1]
				]
			});
		}
	}
	handleMouseUp = () => {
		if (this.mapMouseDownPos) {
			this.mapPosition[0] += this.state.mapMouseMove[0] / 16 / this.mapScale;
			this.mapPosition[1] += this.state.mapMouseMove[1] / 16 / this.mapScale;
			this.mapMouseDownPos = null;
			this.setState({ mapMouseMove: [0, 0] });
			this.draw();
		}
	}

	handleWheel = event => {
		let mapScaleNew = this.mapScale * 1 + event.deltaY / 100;
		if (mapScaleNew < 100 && mapScaleNew > 0.125) {
			this.mapScale = mapScaleNew;
			this.draw();
		}
	}

	render() {
		return (
			<div>
				<button onClick={this.draw}>draw</button>
				<br />
				<div className="map" style={{ height: this.state.mapSize[0], width: this.state.mapSize[1] }}
					onMouseDown={this.handleMouseDown}
					onMouseMove={this.handleMouseMove}
					onMouseUp={this.handleMouseUp}
					onMouseOut={this.handleMouseUp}
					onWheel={this.handleWheel}
				>
					<canvas
						className="canvas"
						ref={canvas => { this.canvas = canvas; this.canvasContext = canvas ? canvas.getContext("2d") : null; }}
						height={this.state.mapSize[0] * 3}
						width={this.state.mapSize[1] * 3}
						style={{ left: -this.state.mapSize[0] + this.state.mapMouseMove[0], top: -this.state.mapSize[1] + this.state.mapMouseMove[1] }}
					></canvas>
				</div>
			</div >
		);
	}
}

export default maps;