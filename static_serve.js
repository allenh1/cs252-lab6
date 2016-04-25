var http = require("http"),
     fs = require("fs")
port = process.argv[2] || 8888;

function submitCompile() {
    var text = document.getElementById("textareaCode").value;
    var date = new Data();
    var name = "code" + date.getMonth() + "" + date.getHours() + "" +
	date.getMinutes() + "" + date.getFullYear() + ".bc";
    fs.write(name, text, function(err) {
	console.log("Saving:\n"+text);
    });
}
