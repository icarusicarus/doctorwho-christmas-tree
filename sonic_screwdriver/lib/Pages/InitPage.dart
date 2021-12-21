import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import '../Pages/BluetoothPage.dart';
import '../Pages/SonicPage.dart';

class InitPage extends StatefulWidget {
  @override
  _InitPageState createState() => _InitPageState();
}

class _InitPageState extends State<InitPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
        body: Container(
            width: double.infinity,
            height: double.infinity,
            decoration: BoxDecoration(
                image: DecorationImage(
              image: AssetImage('images/tardis.jpg'),
              fit: BoxFit.cover,
            )),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                ElevatedButton(
                    child: const Text('Connect Board'),
                    onPressed: () {
                      Navigator.push(
                          context,
                          MaterialPageRoute(
                              builder: (context) => BluetoothPage()));
                    }),
                ElevatedButton(
                    child: const Text('Sonic Screwdriver'),
                    onPressed: () {
                      Navigator.push(context,
                          MaterialPageRoute(builder: (context) => SonicPage()));
                    })
              ],
            )));
  }
}
