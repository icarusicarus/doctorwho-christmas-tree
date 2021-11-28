import 'package:flutter/material.dart';
import './Pages/InitPage.dart';

Future main() async {
  runApp(new SonicScrewDriver());
}

class SonicScrewDriver extends StatefulWidget {
  @override
  _SonicScrewDriverState createState() => _SonicScrewDriverState();
}

class _SonicScrewDriverState extends State<SonicScrewDriver> {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        title: 'Sonic Screwdriver',
        home: FutureBuilder<String>(
          builder: (context, snapshot) {
            return InitPage();
          },
        ));
  }
}
