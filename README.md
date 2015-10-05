## Microsoft.IoT.Lightning Nuget package
This repository is for generating the Windows Developer Program for IoT Nuget package. This package contains code which is compiled into the maker's application which can be deployed to a board running Microsoft Windows. 

This nuget package depends on Microsoft.IoT.SDKFromArduino, which contains source files written by the Arduino community. Together these packages ensure compatibility with existing sketches running on Microsoft Windows.

##Build the Nuget package
Please download the Nuget command line utility [nuget.exe](http://nuget.org/nuget.exe) into the lightning and arduino-sdk .\source folders.
Run the Nuget package builder from the .\source folder:

~~~
build-nupkg.cmd
~~~

##Configure Visual Studio
You'll want to redirect visual studio's *user templates* to the repository you are working in.
*Tools -> Options*

![Template Config](images/Nuget_TemplateConfig.png)

Under *Projects and Solutions*, select *General*,

###Nuget Package sources

In order to install nuget packages from your local builds, you'll need to add both the lightning nuget and arduino-sdk to the nuget package manager sources. Following the below instructions for each sdk source:

For Visual Studio Express, nativate to *Tools -> Nuget Package Manager -> Package Manager Settings*
For Other editions of Visual Studio, nativate to *Tools -> Library Package Manager -> Package Manager Settings*

![Package Config](images/Nuget_PackageSourceConfig_VS2015.png)


1. Click the "+" button to add a new source
1. Set the name to something descriptive
1. Click the "..." button and navigate to your local sources directory (.\source folder)
1. Click the "Update" button to save the Package Sources changes

###Nuget Package Manager

In order to install prerelease (current) version of Lighning as well as receive prerelease updates to the Lightning package, make sure to set the "Include prerelease" option in the Nuget Package Manager.

![Package Config](images/Nuget_PackageManager.PNG)

1. Right click References in your project
1. Click "Manager Nuget Packages..."
1. Select package sources for Lightning nuget
1. Click "Include prerelease".

