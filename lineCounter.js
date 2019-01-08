const fs = require('fs');

function GetFileList(path){
	toScan = fs.readdirSync(path);
	for (let i=0; i<toScan.length; i++){
		toScan[i] = path+"/"+toScan[i];
	}

	let files = [];

	for (let i=0; i<toScan.length; i++){
		let stats = fs.lstatSync(toScan[i]);

		if (stats.isFile()){
			files.push(toScan[i]);
			continue;
		}

		let nxt = fs.readdirSync(toScan[i]);
		for (let j=0; j<nxt.length; j++){
			toScan.push( toScan[i] + '/' + nxt[j] );
		}
	}


	return files;
}


function CountChars(file){
	data = fs.readFileSync(file, 'utf8');

	let result = {lines: 0, chars: data.length};
	for (let i=0; i<data.length; i++){
		if (data[i] == "\n"){
			result.lines++;
		}
	}

	return result;
}


let allFiles = ['source/flags.hpp', 'source/main.cpp'].concat(GetFileList('source/lib'));

let lines = 0;
let chars = 0;
for (let file of allFiles){
	let temp = CountChars(file);
	lines += temp.lines;
	chars += temp.chars;
}

console.log(`lines      :   ${ lines }`);
console.log(`characters :   ${ chars }`);
console.log(`words      : ~ ${ Math.floor(chars/5.1) }`)
