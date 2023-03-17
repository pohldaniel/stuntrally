# stuntrally 2.70

I used boost 1.81.0 https://www.boost.org/users/history/version_1_81_0.html.

Out of the "VS Native Command Prompt"

Change the Directory to Downloaddirectory of Boost with "cd boost_1_81_0", type

  bootstrap.bat

and depending on the choosen "VS Native Command Prompt" the build will be x64 or x86

&emsp;b2 install variant=release runtime-link=shared address-model=64 --prefix=C:\stuntrally\libboost (Md build I used this) <br />
&emsp;b2 install variant=release runtime-link=static address-model=64 --prefix=C:\stuntrally\libboost (Mt build) <br />

&emsp;b2 install variant=debug runtime-link=shared address-model=32 --prefix=C:\stuntrally\libboost (Md build) <br />
&emsp;b2 install variant=debug runtime-link=static address-model=32 --prefix=C:\stuntrally\libboost (Mt build) <br />
  
It will be necessary to rename the libboost/lib folder to libboost/x64 or libboost/x84. Maybe reanaming of the .lib is also necessary, it depends on the used Visual Studio Version. With Visual Studio 2015 the needed libs looks like this.

&emsp;libboost_chrono-vc140-mt-x64-1_81.lib<br />
&emsp;libboost_filesystem-vc140-mt-x64-1_81.lib<br />
&emsp;libboost_thread-vc140-mt-x64-1_81.lib<br />
&emsp;libboost_wave-vc140-mt-x64-1_81.lib<br />
