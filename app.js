'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
  // console.log(uploadFile.name);
  let filenameSplit = uploadFile.name.split(".");
  const fs = require('fs');
  var fileNames = fs.readdirSync('uploads');
  let unique = 1;

  for (let i = 0; i < fileNames.length; i++) {
    let filenameSplit2 = fileNames[i].split(".");
    if (filenameSplit[0] == filenameSplit2[0]) {
      unique = 0;
    }
  }

  if (unique == 0) {
    console.log("File already exists");
  } else {
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
      if(err) {
        return res.status(500).send(err);
      }
  
      res.redirect('/');
    });
  }
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

let sharedLib = ffi.Library('libsvgparser', {
  'fileSVGtoJSON': ['string', ['string', 'string']],
  'svgViewPannel': ['string', ['string']],
  'getOtherAttributes': ['string', ['string', 'string', 'int']],
  'createSVGfromJSON': ['int', ['string', 'string']],
  'JSONAddComponent': ['int', ['string', 'string', 'int']],
  'scaleShapes': ['int', ['string', 'string', 'float']],
});

//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.data1 + " " + req.query.data2;
 
  res.send(
    {
      somethingElse: retStr
    }
  );
});

app.get('/getFileLog', function(req , res){
  const fileLogObject = new Object();
  const fs = require('fs');
  var fileNames = fs.readdirSync('uploads');
  let svgShapeCounts;
  let parsedSvgShapeCounts;

  for (let i = 0; i < fileNames.length; i++) {
    var stats = fs.statSync("uploads/"+fileNames[i]);
    svgShapeCounts = sharedLib.fileSVGtoJSON("uploads/"+fileNames[i], "svg.xsd");
    parsedSvgShapeCounts = JSON.parse(svgShapeCounts);
    // console.log(parsedSvgShapeCounts);
    
    let file = new Object();
    file['name'] = fileNames[i];
    file.size = Math.round(stats.size/1000);
    file.numRects = parsedSvgShapeCounts.numRect;
    file.numCircles = parsedSvgShapeCounts.numCirc;
    file.numPaths = parsedSvgShapeCounts.numPaths;
    file.numGroups = parsedSvgShapeCounts.numGroups;
    fileLogObject['file'+i] = file;
  }

  var myJsonString = JSON.stringify(fileLogObject);

  res.send(myJsonString);
});

app.get('/getViewPannel', function(req , res){
  let SVGinfo = sharedLib.svgViewPannel("uploads/"+req.query.filename);
  console.log("AJAX data:");
  console.log(SVGinfo);

  res.send(SVGinfo);
});

app.get('/getOtherAttributes', function(req , res){
  let otherAttributes = sharedLib.getOtherAttributes("uploads/"+req.query.filename, req.query.shapeType, req.query.componentNum);
  console.log("AJAX data:");
  console.log(otherAttributes);
  res.send(otherAttributes);
});

app.get('/createSVG', function(req , res){
  let filenameSplit = req.query.filename.split(".");
  const fs = require('fs');
  var fileNames = fs.readdirSync('uploads');

  let unique = 1;
  for (let i = 0; i < fileNames.length; i++) {
    let filenameSplit2 = fileNames[i].split(".");
    if (filenameSplit[0] == filenameSplit2[0]) {
      unique = 0;
    }
  }

  if (req.query.title == "") {
    req.query.title = " ";
  }
  if (req.query.description == "") {
    req.query.description = " ";
  }
  let string = "{\"title\":\""+req.query.title+"\",\"descr\":\""+req.query.description+"\"}";
  let filename = "uploads/"+req.query.filename;

  let valid = 0;
  if (filenameSplit[1] == "svg" && unique == 1) {
    valid = sharedLib.createSVGfromJSON(string, filename);
  } else {
    valid = 0;
  }

  let returnval;

  if (unique == 0) {
    returnval = "not unique";
  } else if (valid == 1) {
    returnval = "valid";
  } else if (valid == 0) {
    returnval = "invalid";
  }

  console.log("AJAX data:");
  console.log(returnval);
  res.send(returnval);
});

app.get('/addRect', function(req , res){
  let string = "{\"x\":\""+req.query.x+"\",\"y\":\""+req.query.y+"\",\"w\":\""+req.query.width+"\",\"h\":\""+req.query.height+"\",\"units\":\""+req.query.units+"\"}";

  let filename = "uploads/"+req.query.filename;
  let valid = sharedLib.JSONAddComponent(string, filename, 2);
  let returnval;

  if (valid == 1) {
    returnval = "valid";
  } else if (valid == 0) {
    returnval = "invalid";
  }

  console.log("AJAX data:");
  console.log(returnval);
  res.send(returnval);
});

app.get('/addCirc', function(req , res){
  let string = "{\"cx\":\""+req.query.cx+"\",\"cy\":\""+req.query.cy+"\",\"r\":\""+req.query.r+"\",\"units\":\""+req.query.units+"\"}";

  let filename = "uploads/"+req.query.filename;
  let valid = sharedLib.JSONAddComponent(string, filename, 1);
  let returnval;

  if (valid == 1) {
    returnval = "valid";
  } else if (valid == 0) {
    returnval = "invalid";
  }

  console.log("AJAX data:");
  console.log(returnval);
  res.send(returnval);
});

app.get('/scaleShape', function(req , res){
  let filename = "uploads/"+req.query.filename;
  let returnval;
  let valid = sharedLib.scaleShapes(filename, req.query.shapeType, req.query.scaleFactor);

  if (valid == 1) {
    returnval = "valid";
  } else if (valid == 0) {
    returnval = "invalid";
  }

  console.log("AJAX data:");
  console.log(returnval);
  res.send(returnval);
});

app.get('/editTitle', function(req , res){
  res.send("This is the function stub for editing the title");
});

app.get('/editDescription', function(req , res){
  res.send("This is the function stub for editing the description");
});

app.get('/editAttribute', function(req , res){
  res.send("This is the function stub for editing an attribute");
});

app.get('/addAttribute', function(req , res){
  res.send("This is the function stub for adding an attribute");
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
