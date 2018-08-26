const os = require("os");
const fs = require("fs");
const cp = require('child_process');
const log = require("./js/pktlog.js");
const main = require("./js_v3/main.js");
const Version = require("./js_v3/version.js");
const Depend = require("./js_v3/depend.js");
const Update = require("./js_v3/update.js");
const Compile = require("./js_v3/compile.js");
const Bale = require("./js_v3/bale.js");
const Publish = require("./js_v3/publish.js");
const Bale_Customize = require("./js_v3/bale_customize.js")
const Sln = require("./js_v4/sln.js")

try {
	log.clean();

	main.initHelper("help/h/?","显示当前帮助文档");
	main.registCmdHandle("version/v",new Version(),"更新项目版本号");
	main.registCmdHandle("depend/d",new Depend(),"更新项目依赖库");
	main.registCmdHandle("update/u",new Update(),"发布项目依赖库");
	main.registCmdHandle("compile/c",new Compile(),"编译工程项目");
	main.registCmdHandle("bale/b",new Bale(),"打包工程项目");
	main.registCmdHandle("balecustomize/bc",new Bale_Customize(),"打包用户自定义工程项目");
	main.registCmdHandle("publish/p/",new Publish(),"发布工程项目");
	main.registCmdHandle("sln/s",new Sln(),"根据解决方案依赖项更新项目依赖");

	var arg = process.argv.splice(2);

	main.doCmdHandle(arg[0] ? arg[0]:"",arg);
} catch (error) {
	log.log(error);
}
