LapTime.osax
============

LapTime.osax is deprecated.
Because scripting additions of third parties are not supported in macOS 10.14 or later.

Use AppleScript library [LapTime.scptd] instead of LapTime.osax.

[Laptime.scptd]: https://www.script-factory.net/XModules/LapTime/en/index.html

LapTime.osax is a scripting addition to measure execution time of a script of AppleScript.
It is useful to evaluate performance of the script and to tune up the script.
You can measure execution time by using current date command which is AppleScript's built-in command.
But time resolution of AppleScript's date class is 1 sec, which is too rough for performance measurements.
The time resolution of LapTime.osax is under a few millisecond.


## Home page
English :
* https://www.script-factory.net/graveyard/osax/LapTime/en/index.html

Japanese :
* https://www.script-factory.net/software/terminal/OpenInTerminal/index.html

## Building
Requirements :
* OS X 10.9 or later
* Xcode
* [ModuleLoader]

[ModuleLoader]: https://www.script-factory.net/XModules/ModuleLoader/en/index.html

## License

Copyright &copy; 2010-2016 Kurita Tetsuro
Licensed under the [GPL license][GPL].
 
[GPL]: http://www.gnu.org/licenses/gpl.html

