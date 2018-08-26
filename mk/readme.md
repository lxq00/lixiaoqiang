#VGSII Git一键编译打包工具介绍

>存放路径：`\\192.168.0.10\个人命令\l李小强\__VGSII___Compile\vgsii_git_pkt_v1.0.zip`


#####修改历史
```
版本号		时间				作者		修改内容 
V1.0 	  2016年7月14日	     李小强		 新建		
```
--
###一：功能说明

Git自动打包工具实现对基础库、服务组件、设备组件、Decoder进行自动编译和打包，自动对依赖库进行下载。

基础库:自动进行编译（2012_Realse、2012_Debug、2008_Realse、2008_Debug）,并对打包输出的库和依赖的头文件进行打包，并上传到指定目录;	

组件：自动下载指定版本的依赖库，并对代码进行编译(2012_Realse、2012_Debug),并将输出的组件服务包(Realse、Debug)、并自动修改组件配置文件

>上述指定目录暂为`\\192.168.0.10\个人目录\l李小强\__VGSII___Compile`

###二：工具介绍

打包工具ZIP中包括的目录有install、mk；文件包括`Makefile、pkgcfg.json、prjcfg.json、Publish.bat、Publish.sh、RIS.mk、beforeCompile.bat、beforeCompile.sh、packIng.bat、packIng.sh、buildVersion.bat、buildVersion.sh、Version.tpl、WinVersion.tpl`

`install目录`：存放打包需要的基础文件和依赖文件的目录  
`install->common目录`：存放打包需要的功能文件，包括Windows包和Linux包，常常存放服务组件说明文件、XMQ配置文件、readme.mk等于Windows和Linux环境下运行无关的文件  
`install->win32目录`：存放Windows环境下运行依赖的库文件或其他文件  
`install->x86目录`：存放Linux环境下运行依赖的库文件或其他文件  
`mk目录`：存放有打包编译等相关的功能基础文件。
	
`beforeCompile.bat/sh文件`：为在自动编译之前需要执行自定义的特殊处理  
`Makefile文件`：在Linux环境下编译所需要的编译文件  
`packIng.bat/sh文件`：为在打包过程中需要执行自定义的特殊处理，调用方式为“packIng.bat debug _pack_tmp_debug” debug为当前打包的类型，_pack_tmp_debug当前打包生产的临时目录  
`pkgcfg.json文件`：打包工程配置文件，配置打包功能的相关信息，具体介绍参考后续详细说明  
`prjcfg.json文件`：打包项目配置文件，配置打包项目的相关信息，具体介绍参考后续详细说明  
`RIS.mk文件`：为Linux下编译所需的配置文件，RIS名称可以修改，但需要和Makefile文件中进行对应  
`buildVersion.bat/sh文件`：该文件为生产项目版本信息文件脚本，具体介绍参考后续版本号章节  
`Version.tpl文件`：该文件为配置工程大版本号的配置文件，配置大版本号如2.3.0  
`WinVersion.tpl文件`：该文件为配置项目版本信息描述等生产版本信息的说明文件  
>注：同名文件的bat和sh只是对windows和linux环境支持的相同功能脚本。

###三：配置介绍
####1：版本号配置文件【必填】
Version.tpl：一个仓库一个配置文件，后续自动打包回自动生成，前期人工管理，管理仓库版本号的前3位。  
WinVersion.tpl：一个项目一个配置文件，配置项目的版本信息描述，必须`ANSI编码`。  
####2：编译配置文件【必填】
pkgcfg.json：一个仓库一个配置文件，配置工程的依赖等相关信息  
prjcfg.json：一个项目一个配置文件，配置项目的打包编译等相关信息  
####3：编译控制文件【可选】
beforeCompile.bat/sh：控制编译器前的特殊处理  
packIng.bat/sh：控制打包前的特殊处理  
###4：Linux编译配置文件【可选】
Makefile：一个仓库一个编译文件，配置需要编译的工程  
RIS.mk：一个项目一个，配置相关相关信息  

###四：使用方法
####1：Git仓库目录介绍
在Git一个仓库下可以存放单个项目工程（vgsii_ris），也可以存放多个项目工程（vgsii_storage）  
在项目工程目录下需存在以下文件夹  
  Lib：该项目依赖的除基础库以为的其他库，或该项目生成的库文件  
  Bin：该项目生成的可执行程序，Bin/win32 windows目录，Bin/x86 Linux目录。
####2：准备功能
安装依赖文件  
Windows依赖文件为node-v6.2.0-x64.msi、Linux依赖文件为node-v6.2.0-x64.msi  
>存放路径"\\192.168.0.10\个人目录\l李小强\__VGSII___Compile"

###3：安装工具
Git仓库下只有一个项目  
将打包工具内的所有文件覆盖到项目根目录下。`  

Git仓库下存在多个项目  
1.将打包工具中的mk目录、pkgcfg.json、Publish.bat、Publish.sh存放到仓库跟目。   
2.将打包工具中的mk目录、Makefile、prjcfg.json、RIS.mk、beforeCompile.bat、beforeCompile.sh、packIng.bat、packIng.sh存放到项目根目录。

####4：配置Linux编译
在项目录下将RIS.mk修改为自己项目的名称，修改RIS.mk的源码目录和项目名称，如有需要请复制粘贴多个mk文件。  
修改Makefile文件，将修改的mk文件添加到Makefile中。  
>Linux下手动编译命令 make，清除编译数据重新编译命令 make clean。

####5：配置自动编译打包
1.修改仓库目录下的pkgcfg.json文件，该文件内容如下：  

```
{
   	"Depends":[ //依赖项目设置
		{"name":"vgsii_public","version":"20160712"} //表示依赖vgsii_public项目版本号为20160712
   	],
   	"Project":[	//表示该仓库下存在存在的项目 如一个项目内容为空
		"RNode",
   	]
}
```	  
2.修改项目目录下prjcfg.json文件，该文件内容如下：

```	
{
   	"ProjectName":"vgsii_ris",	//该项目的名称
   	"ProjectOutput":"RIS",		//该项目输出的exe名称
   	"Compile":{	//编译选项配置
       	"Win32":{
          "vs2008":"Solution_2008.sln",//需要编译的解决方案
           	"vs2012":"Solution_2012.sln"//需要编译的解决方案
       	}
   	},
   	"Package":{ //打包配置，默认打包后上传到指定目录
       	"Library":{	//打包成library 基础库才需要
           	"Valid":"false",	//是否需要
           	"Exclude":[	//需要排除那些include和lib不打包
				"boost"
           	]
       	},
       	"App":{	//打包应用程序，打包组件选择
           	"Valid":"true",	//是否需要
           	"Depends":[		//打包该包需要哪些其他依赖库，Linux下无效
				"Base",
				"XMQ",
				"XMQClient",
				"XML"
           	],
			"Publish":"true"	//是否需要上传到指定目录
       	}
   	}
}
```
3.配置编译所需的特殊命令beforeCompile.bat/sh,如不需要，可以删除。  
```该脚本作用为在编译前除依赖库外的其他库进行解压/拷贝等操作```
	
4.配置打包所需的特殊命令packIng.bat/sh,如不需要，可以删除。
```该脚本为打包中需要处理的特殊处理，如拷贝打包工具无法拷贝的其他库等```
	
>注：打包工具只会自动拷贝Bin、Lib、_Output\Lib目录下的库和exe，拷贝目标为"依赖文件.dll"或"依赖文件_debug.dll"等命名规则的文件。

####6：自动编译打包
运行仓库命令下的Publish.bat/sh进行一键编译和打包。  
执行该命令未提示错误表示打包成功。
###五：编译过程文件
在打包过程中如果错误停止时，将会在项目目录下输出编译的过程文件；  
windows为“2012_debug_output.txt”、“2012_realse_output.txt”等文件，Linux为“make_output.txt”,过程文件输出为编译过程错误。
