import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

class SonicPage extends StatefulWidget {
  @override
  _SonicPageState createState() => _SonicPageState();
}

class _SonicPageState extends State<SonicPage> {
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
          onTap: () {},
          child: Image.asset(
            'images/sonic_10th.png',
            fit: BoxFit.cover,
          ),
        )));
  }
}
