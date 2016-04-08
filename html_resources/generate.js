const fs = require('fs');
const rootDataDir = "webpage";
const outputFile = "./output_includes/web_data.h";
const headerFileLocation = "./template_web_data.h.txt";

var files = walkSync(rootDataDir);
var currentFileName = "";
var currentFileLength = -1;
var encoding = "";

var outputString = "";

var headerFile = fs.readFileSync(headerFileLocation, 'utf8');

String.prototype.replaceAll = function(search, replacement) {
    var target = this;
    return target.replace(new RegExp(search, 'g'), replacement);
};

for (var i = 0; i < files.length; i++){
	currentFileName = files[i];
	encoding = getEncoding(currentFileName);
	outputString += "\n\n\n" + currentFileName + "\nEncoding: " + encoding + " Length:  ";
	var currentData = gotData(fs.readFileSync(rootDataDir + "/" + currentFileName, encoding));

	headerFile = headerFile.replaceAll("##" + currentFileName + "-data##", currentData);
	headerFile = headerFile.replaceAll("##" + currentFileName + "-length##", currentFileLength);
	headerFile = headerFile.replaceAll("##" + currentFileName + "-length-digits##", ("" + currentFileLength).length);
	headerFile = headerFile.replaceAll("##" + currentFileName + "-length_as_array##", formatToArray(currentFileLength));
}

console.log(outputString);

var file = fs.openSync(outputFile, 'w');
fs.writeFile(outputFile, headerFile, function(err) {
    if(err) {
        return console.log(err);
    }

    console.log("The file was saved!");
});

function getEncoding(file){
	if (currentFileName.indexOf(".") < 0){
		return 'hex';
	}
	var extention = file.substring(file.lastIndexOf("."), file.length);
	extention = extention.toLowerCase();
	switch (extention){
		case ".html":
		case ".htm":
		case ".txt":
		case ".css":
		case ".js":
			return 'utf8'
		default:
			return 'hex'
	}

}

function gotData(data){
	currentFileLength = data.length;
	outputString += currentFileLength + "\n\n\n";
	if (encoding == 'utf8'){
		outputString += '"' + excape(data) + '";';
		return excape(data);
	} else {
		outputString += "{\n" + outputHexArray(data) + "\n};";
		return outputHexArray(data);
	}
}

// source https://gist.github.com/kethinov
// List all files in a directory in Node.js recursively in a synchronous fashion
function walkSync(dir, filelist, path) {
	path = path || "";
	var files = fs.readdirSync(dir);
	filelist = filelist || [];
	files.forEach(function(file) {
		if (fs.statSync(dir + '/' + file).isDirectory()) {
			filelist = walkSync(dir + '/' + file, filelist, path + file + "/");
		}
		else {
			filelist.push(path + file);
		}
	});
	return filelist;
};

function formatToArray (string){
	var output = "'";
	var seperator = "', '";
	string = "" + string;
	for (var a = 0; a < string.length - 1; a++){
		output += string.charAt(a) + seperator;
	}
	return output + string.charAt(string.length - 1) + "',";
}

// source https://github.com/joliss/js-string-escape
function excape (string) {
	return ('' + string).replace(/["'\\\t\n\r\u2028\u2029]/g, function (character) {
		// Escape all characters not included in SingleStringCharacters and
		// DoubleStringCharacters on
		// http://www.ecma-international.org/ecma-262/5.1/#sec-7.8.4
		switch (character) {
			case '"':
			case "'":
			case '\\':
				return '\\' + character
			// Four possible LineTerminator characters need to be escaped:
			case '\n':
				return '\\n'
			case '\t':
				return '\\t'
			case '\r':
				return '\\r'
			case '\u2028':
				return '\\u2028'
			case '\u2029':
				return '\\u2029'
		}
  	});
}
function outputHexArray (string) {
	var returnString = "";
	for (var a = 0; a < string.length; a += 2){
		returnString += "0x" + string.substring(a, a + 2);

		if (a + 2 != string.length) {
			returnString += ", ";
		}
		if ((( (a - 22) % 24) == 0)&&(a != 0) && (a + 2 != string.length)){
			returnString += "\n";
		}
	}
	return returnString;
}