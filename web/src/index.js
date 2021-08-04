import React from 'react';
import ReactDOM from 'react-dom';


import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap/dist/js/bootstrap.js';
import 'bootstrap/dist/js/bootstrap.bundle'
import './index.css';

import { Serial } from './serial.js';

global.jQuery = require('jquery');
require('bootstrap');

class Arcade extends React.Component {

    ArcStateConnect = "connect";
    ArcStateConnecting = "connecting";
    ArcStateIdle = "idle";
    ArcStateGame = "game";
    ArcStateOver = "over";

    constructor(props) {
        super(props);
        this.state = {
            player1: 0,
            player2: 0,
            time: 0,
            arcade: this.ArcStateConnect,
        }
    }

    handleConnectClick() {
        this.serial = new Serial();
        this.setState({
            arcade: this.ArcStateConnecting
        });
        this.serial.getDevice().then(() => {
            console.log("Serial connected, updating status.");
            this.setState({
                arcade: this.ArcStateIdle
            });
            this.readArcade();
        }).catch(c => {
            console.log("Catch");
            this.setState({
                arcade: this.ArcStateConnect
            });
        });
    }

    readArcade() {

        this.serial.read().then(result => {
            if ((result.length > 5) && (result[result.length - 1] === "\n")) {
                const line = result.split("\n")[0];
                console.log(line);
                const words = line.split(" ");
                if (words[0] === "IDLE") {
                    this.setState({
                        arcade: this.ArcStateIdle,
                        time: words[1],
                    });
                } else if (words[0] === "GAME_START") {
                    this.setState({
                        arcade: this.ArcStateGame,
                        time: words[1],
                    });
                } else if (words[0] === "STATUS") {
                    console.log("STATUS");
                    if ((this.state.time !== words[1]) || (this.state.player1 !== words[2]) || (this.state.player2 !== words[3])) {
                        console.log("UPDATE");
                        this.setState({
                            arcade: this.ArcStateGame,
                            time: words[1],
                            player1: words[2],
                            player2: words[3]
                        });
                    }
                } else if (words[0] === "GAME_OVER") {
                    console.log("GAME OVER");
                    this.setState({
                        arcade: this.ArcStateOver,
                        time: 0,
                        player1: words[1],
                        player2: words[2]
                    });
                }
            }

            setTimeout(() => {
                this.readArcade();
            }, 80);
        });
    }

    render() {
        if (navigator.serial) {
            if (this.state.arcade === this.ArcStateConnect) {
                return (
                    <div className="connect">
                        <button onClick={(e) => this.handleConnectClick()} className="btn btn-lg btn-secondary">Connect</button>
                        <br />
                        <small>Version: 0.1</small>
                    </div>
                )
            } else if (this.state.arcade === this.ArcStateConnecting) {
                return (
                    <div className="connect">
                        <h1>Connecting...</h1>
                    </div>
                )

            } else if (this.state.arcade === this.ArcStateGame) {

                return (
                    <div className="connect">
                        <div className="container">
                            <h1>{this.state.time}</h1>

                            <div className="row">
                                <div className="col-6">
                                    <p className="score">{this.state.player2}</p>
                                </div>
                                <div className="col-6">
                                    <p className="score">{this.state.player1}</p>
                                </div>

                            </div>
                        </div>

                    </div>
                )
            } else if (this.state.arcade === this.ArcStateIdle) {
                return (
                    <div className="connect">
                        <div className="container">
                            <h3>Hit start to begin</h3>
                        </div>
                    </div>)
            } else if (this.state.arcade === this.ArcStateOver) {
                return (
                    <div className="connect">
                        <div className="container">
                            <h2>Game Over!</h2>

                            <div className="row">
                                <div className="col-6">
                                    <p className="score">{this.state.player2}</p>
                                </div>
                                <div className="col-6">
                                    <p className="score">{this.state.player1}</p>
                                </div>

                            </div>
                        </div>

                    </div>
                )
            }
        } else {
            return (
                <h2>Sorry, your browser does not support Web Serial!</h2>
            )
        }
    }
}

// ========================================

ReactDOM.render(
    <Arcade />,
    document.getElementById('root')
);
