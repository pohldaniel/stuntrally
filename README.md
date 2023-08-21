# Stuntrally 2.7.0 VDrift 2023-03-05

This repository contains a Visual Studio 2015 setup of https://github.com/stuntrally/stuntrally, https://github.com/VDrift/vdrift and thre dependencies. After compiling keep in mind the tools folder for setting up the Release Folder. The build process is just tested with Release x64 configuration. It should be clear that the Copyright of Stuntrally, VDrift and of it's dependencies are not in my hand.

My intention for this repository is to get a deeper understanding of game programing.

Used major libraries <br/>

&emsp;https://github.com/OGRECave/ogre/releases/tag/v13.5.3<br/>
&emsp;https://freeimage.sourceforge.io/download.html<br/>
&emsp;MyGui 3.4.0 http://mygui.info<br/>
&emsp;https://github.com/bulletphysics/bullet3/releases/tag/3.25<br/>
&emsp;SDL2 2.0.22 https://www.libsdl.org<br/>
&emsp;https://github.com/assimp/assimp/releases/tag/v5.2.5<br/>
&emsp;https://www.boost.org/users/history/version_1_81_0.html.5<br/>

**Installing Boost:** Out of the "VS Native Command Prompt"

Change the Directory to the downloaddirectory of Boost with "cd boost_1_81_0", type

  &emsp;bootstrap.bat

and depending on the choosen "VS Native Command Prompt" the build will be x64 or x86

&emsp;b2 install variant=release runtime-link=shared address-model=64 --prefix=C:\stuntrally\libboost (Md build I used this) <br/>
&emsp;b2 install variant=release runtime-link=static address-model=64 --prefix=C:\stuntrally\libboost (Mt build) <br/>

&emsp;b2 install variant=debug runtime-link=shared address-model=32 --prefix=C:\stuntrally\libboost (Md build) <br/>
&emsp;b2 install variant=debug runtime-link=static address-model=32 --prefix=C:\stuntrally\libboost (Mt build) <br/>
  
It will be necessary to rename the libboost/lib folder to libboost/lib/x64 or libboost/lib/x84. Maybe reanaming of the .lib is also necessary, it depends on the used Visual Studio Version. With Visual Studio 2015 the needed libs looks like this.

&emsp;libboost_chrono-vc140-mt-x64-1_81.lib<br/>
&emsp;libboost_filesystem-vc140-mt-x64-1_81.lib<br/>
&emsp;libboost_thread-vc140-mt-x64-1_81.lib<br/>
&emsp;libboost_wave-vc140-mt-x64-1_81.lib<br/>

**Attention:** The DirectX Include path form the Project "RenderSystem_DirectD3D11" depends on the choosen Windows Kit. In my case I used Version 8.1 but I recommand to go with a newer one.<br/>

&emsp;C:\Program Files (x86)\Windows Kits\8.1\Include\um
