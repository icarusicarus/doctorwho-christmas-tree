import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:vibration/vibration.dart';
import 'package:assets_audio_player/assets_audio_player.dart';

class SonicPage extends StatefulWidget {
  @override
  _SonicPageState createState() => _SonicPageState();
}

class _SonicPageState extends State<SonicPage> {
  AssetsAudioPlayer audioPlayer = AssetsAudioPlayer();
  @override
  void initState() {
    audioPlayer.open(Audio('audio/sonic_screwdriver_sound.mp3'),
        autoStart: false);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          backgroundColor: Colors.transparent,
          elevation: 0.0,
          leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.black),
            onPressed: () => Navigator.of(context).pop(),
          ),
        ),
        body: Center(
            child: GestureDetector(
          onTap: () {
            Vibration.vibrate(pattern: [500, 1000, 500, 1000]);
            audioPlayer.play();
          },
          child: Image.asset(
            'images/sonic_10th.png',
            fit: BoxFit.cover,
          ),
        )));
  }
}
