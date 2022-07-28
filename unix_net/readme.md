  一、下载源代码及相应头文件和库文件
  下载链接： git clone git@github.com:unpbook/unpv13e.git
 
  二、解压unpv13e.tar.gz：
 
  tar -xvf unpv13e.tar.gz
 
  三、依次执行以下命令编译代码得到一个libunp.a库文件：
 
  cd unpv13e
  ./configure
  cd lib
  make
 
  四、复制文件libunp.a到usr/src目录下（其他目录也可以，仅以示例）：
 
  sudo cp libunp.a /usr/lib
 
  五、在lib目录中找到unp.h和config.h两个头文件复制到与源代码同目录下
  （其他路径也可以，但需要修改源文件中引用unp.h和config.h头文件的路径，仅以示例）；
 
  六、编译程序时要以-lunp参数方式指定程序需要依赖的库（unplib.a），
  如编译daytimetcpcli.c文件：
 
 gcc daytimetcpcli.c -o daytimetcpcli -lunp
 
  七、运行程序。