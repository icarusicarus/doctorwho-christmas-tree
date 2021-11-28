import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import '../Pages/SonicPage.dart';

class InitPage extends StatefulWidget {
  @override
  _InitPageState createState() => _InitPageState();
}

class _InitPageState extends State<InitPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
        body: Center(
            child: ElevatedButton(
                child: const Text('Sonic Screwdriver'),
                onPressed: () {
                  Navigator.push(context,
                      MaterialPageRoute(builder: (context) => SonicPage()));
                })));
  }
}
