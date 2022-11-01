jQuery(document).ready(function() {
    // On page-load AJAX Example
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFileLog',   //The server endpoint we are connecting to
        success: function (data) {
            console.log("Loading up the file log pannel and drop-down lists...");
            let SVGList = document.getElementsByClassName("SVGList");
            // console.log(SVGList);
            // console.log(data);
            if (Object.entries(data).length != 0) {
                // load the file log table
                let fileLogTable = document.getElementById("fileLogTable");
                fileLogTable.innerHTML = `
                <tr>
                <th>Image (click to download)</th>
                <th>File name (click to download)</th>
                <th>File size</th>
                <th>Number of Rectangles</th>
                <th>Number of Circles</th>
                <th>Number of Paths</th>
                <th>Number of Groups</th>
                </tr>`;

                for (const [key, value] of Object.entries(data)) {
                    if (value.numRects != undefined || value.numCircles != undefined || value.numPaths != undefined || value.numGroups != undefined) {
                        fileLogTable.innerHTML += `
                        <tr>
                            <td class="fileLogImage"><a href="../uploads/`+value.name+`" download><img src="../uploads/`+value.name+`" alt="`+value.name+`"></a></td>
                            <td><a href="../uploads/`+value.name+`" download>`+value.name+`</a></td>
                            <td>`+value.size+`kb</td>
                            <td>`+value.numRects+`</td>
                            <td>`+value.numCircles+`</td>
                            <td>`+value.numPaths+`</td>
                            <td>`+value.numGroups+`</td>
                        </tr>`;
        
                        for (let i = 0; i < SVGList.length; i++) {
                            SVGList[i].innerHTML += `<option value="`+value.name+`">`+value.name+`</option>`;
                        }
                    }
                }
            } else {
                let fileLogTable = document.getElementById("fileLogTable");
                fileLogTable.innerHTML = "<tr><th><h3>No files</h3></th></tr>";
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log("Failed to load up the file log pannel and drop-down lists...");
            console.log(error); 
        }
    });
});

document.getElementById("chooseSVGBtn").addEventListener('click', function(e) {
    let filename = document.getElementsByClassName("SVGList")[0].value;
    // On page-load AJAX Example
    if (document.getElementById("svgTableUpper")) {
        document.getElementById("SvgViewPannel").removeChild(document.getElementById("svgTableUpper"));
    }
    if (document.getElementById("svgTableLower")) {
        document.getElementById("SvgViewPannel").removeChild(document.getElementById("svgTableLower"));
    }

    if (filename == "") {
        alert("No filename selected");
    }

    if (filename != "") {
        document.getElementById("SvgViewPannelImg").innerHTML = `<img src="../uploads/`+filename+`" alt="">`;
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'text',       //Data type - we will use JSON for almost everything 
            url: '/getViewPannel',   //The server endpoint we are connecting to
            data: {
                filename: filename,
            },
            success: function (data) {
                if (data == "[]") {
                    console.log("There was an error loading up the SVG view pannel");
                } else {
                    console.log("Loading up the SVG view pannel...");
                    // console.log(data);
                    data = data.replace(/(\r\n|\n|\r)/gm, "");
                    data = JSON.parse(data);

                    let svgTableUpper = document.createElement("table");
                    svgTableUpper.classList.add("table");
                    svgTableUpper.classList.add("tableFormat");
                    svgTableUpper.setAttribute("id", "svgTableUpper");
                    // if (data[0][0].title != " " || data[0][0].description != " ") {
                    //     svgTableUpper.innerHTML = `
                    //     <tr>
                    //         <th>Title</th>
                    //         <th>Description</th>
                    //     </tr>
                    //     <tr class="SVGComponent">
                    //         <td id="titleSection">`+data[0][0].title+`<br><div class="smallBtn" id="titleEditBtn">Edit Title</div></td>
                    //         <td id="descriptionSection">`+data[0][0].description+`<br><div class="smallBtn" id="descEditBtn">Edit Description<div></td>
                    //     </tr>`;
                    // } else {
                    //     svgTableUpper.innerHTML = `
                    //     <tr>
                    //         <th>Title</th>
                    //         <th>Description</th>
                    //     </tr>
                    //     <tr class="SVGComponent">
                    //         <td id="titleSection">`+data[0][0].title+`<br></td>
                    //         <td id="descriptionSection">`+data[0][0].description+`<br></td>
                    //     </tr>`;
                    // }
                    svgTableUpper.innerHTML = `
                    <tr>
                        <th>Title</th>
                        <th>Description</th>
                    </tr>
                    <tr class="SVGComponent">
                        <td id="titleSection">`+data[0][0].title+`<br><div class="smallBtn" id="titleEditBtn">Edit Title</div></td>
                        <td id="descriptionSection">`+data[0][0].description+`<br><div class="smallBtn" id="descEditBtn">Edit Description<div></td>
                    </tr>`;

                    document.getElementById("SvgViewPannel").appendChild(svgTableUpper);

                    let svgTableLower = document.createElement("table");
                    svgTableLower.classList.add("table");
                    svgTableLower.classList.add("tableFormat");
                    svgTableLower.setAttribute("id", "svgTableLower");
                    svgTableLower.innerHTML = `
                    <tr>
                        <th>Component</th>
                        <th>Summary</th>
                        <th>Other Attributes</th>
                    </tr>`;

                    for (let i = 1; i < data.length; i++) {
                        let componentType;

                        for (let j = 0; j < data[i].length; j++) { // list of a component type (e.g. list of rects)
                            let num = j+1;
                            if (i == 1) {
                                componentType = "Rectangle";
                                svgTableLower.innerHTML += `
                                <tr class="SVGComponent">
                                    <td>`+componentType+` `+num+`</td>
                                    <td>x = `+data[i][j].x+data[i][j].units+`, y = `+data[i][j].y+data[i][j].units+`, width = `+data[i][j].w+data[i][j].units+`, height = `+data[i][j].h+data[i][j].units+`</td>
                                    <td>`+data[i][j].numAttr+`<br>
                                        <div class="smallBtn showAttrBtn">Show Attributes</div>
                                    </td>
                                </tr>`;
                            } else if (i == 2) {
                                componentType = "Circle";
                                svgTableLower.innerHTML += `
                                <tr class="SVGComponent">
                                    <td>`+componentType+` `+num+`</td>
                                    <td>x = `+data[i][j].cx+data[i][j].units+`, y = `+data[i][j].cy+data[i][j].units+`, radius = `+data[i][j].r+data[i][j].units+`</td>
                                    <td>`+data[i][j].numAttr+`<br>
                                        <div class="smallBtn showAttrBtn">Show Attributes</div>
                                    </td>
                                </tr>`;
                            } else if (i == 3) {
                                componentType = "Path";
                                svgTableLower.innerHTML += `
                                <tr class="SVGComponent">
                                    <td>`+componentType+` `+num+`</td>
                                    <td>path data = `+data[i][j].d+`</td>
                                    <td>`+data[i][j].numAttr+`<br>
                                        <div class="smallBtn showAttrBtn">Show Attributes</div>
                                    </td>
                                </tr>`;
                            } else if (i == 4) {
                                componentType = "Group";
                                svgTableLower.innerHTML += `
                                <tr class="SVGComponent">
                                    <td>`+componentType+` `+num+`</td>
                                    <td>`+data[i][j].children+` child elements</td>
                                    <td>`+data[i][j].numAttr+`<br>
                                        <div class="smallBtn showAttrBtn">Show Attributes</div>
                                    </td>
                                </tr>`;
                            }
                        }
                    }

                    document.getElementById("SvgViewPannel").appendChild(svgTableLower);
                }    
            },
            fail: function(error) {
                console.log("Failed to load up the SVG view pannel");
                console.log(error); 
            }
        });   
    }
});


let titleSection = document.getElementById("titleSection");
let descriptionSection = document.getElementById("descriptionSection");

document.addEventListener('click', function(e) {
    if (e.target && e.target.id === "titleEditBtn") {
        titleSection = document.getElementById("titleSection");
        console.log("Editing the title");
        let textArea = document.createElement("textarea");
        textArea.classList.add("editText");
        let br = document.createElement("br");
        // console.log(titleSection.childNodes.length);
        if (titleSection.childNodes.length != 2) {
            textArea.innerHTML = titleSection.childNodes[0].textContent;
        }
        let titleCancelEdit = document.createElement("div");
        titleCancelEdit.innerHTML = "Cancel";
        titleCancelEdit.classList.add("smallBtn");
        titleCancelEdit.setAttribute("id", "titleCancelEdit");
        let titleSaveEdit = document.createElement("div");
        titleSaveEdit.innerHTML = "Save";
        titleSaveEdit.classList.add("smallBtn");
        titleSaveEdit.classList.add("titleSaveEdit");
    
        titleSection.innerHTML = '';
        titleSection.appendChild(textArea);
        titleSection.appendChild(br);
        titleSection.appendChild(titleCancelEdit);
        titleSection.appendChild(titleSaveEdit);

        jQuery.ajax({
            type: 'get',
            dataType: 'text',
            url: '/editTitle',
            success: function (data) {
                console.log(data);
            },
            fail: function(error) {
                console.log(error); 
            }
        });
    }
    if (e.target && e.target.id === "descEditBtn") {
        let descriptionSection = document.getElementById("descriptionSection");
        console.log("Editing the description");
        let textArea = document.createElement("textarea");
        textArea.classList.add("editText");
        textArea.innerHTML = descriptionSection.childNodes[0].textContent;
        let br = document.createElement("br");
        let descCancelEdit = document.createElement("div");
        descCancelEdit.innerHTML = "Cancel";
        descCancelEdit.classList.add("smallBtn");
        descCancelEdit.setAttribute("id", "descCancelEdit");
        let descSaveEdit = document.createElement("div");
        descSaveEdit.innerHTML = "Save";
        descSaveEdit.classList.add("smallBtn");
        descSaveEdit.classList.add("descSaveEdit");
    
        descriptionSection.innerHTML = '';
        descriptionSection.appendChild(textArea);
        descriptionSection.appendChild(br);
        descriptionSection.appendChild(descCancelEdit);
        descriptionSection.appendChild(descSaveEdit);

        jQuery.ajax({
            type: 'get',
            dataType: 'text',
            url: '/editDescription',
            success: function (data) {
                console.log(data);
            },
            fail: function(error) {
                console.log(error); 
            }
        });
    }
    if (e.target && e.target.id === "titleCancelEdit") {
        console.log("Cancel editing the title");
        let titleSection = document.getElementById("titleSection");

        titleSection.innerHTML = titleSection.childNodes[0].innerHTML;
        let editBtn = document.createElement("div");
        let br = document.createElement("br");
        editBtn.innerHTML = "Edit Title";
        editBtn.classList.add("smallBtn");
        editBtn.setAttribute("id", "titleEditBtn");
    
        titleSection.appendChild(br);
        titleSection.appendChild(editBtn);
    }
    if (e.target && e.target.id === "descCancelEdit") {
        let descriptionSection = document.getElementById("descriptionSection");
        console.log("Cancel editing the description");

        descriptionSection.innerHTML = descriptionSection.childNodes[0].innerHTML;
        let editBtn = document.createElement("div");
        let br = document.createElement("br");
        editBtn.innerHTML = "Edit Description";
        editBtn.classList.add("smallBtn");
        editBtn.setAttribute("id", "descEditBtn");
    
        descriptionSection.appendChild(br);
        descriptionSection.appendChild(editBtn);
    }
});

/*
find out the type of shape it is
find out what number of that shap they selected (circle 1, circle 2, circle 3...)
loop through the currect arrays attributes, skip over units and numAttr
*/

// let SVGComponents = document.getElementsByClassName("SVGComponent");
document.addEventListener('click', function(e) {
    let SVGComponents = document.getElementsByClassName("SVGComponent");
    for (let i = 1; i < SVGComponents.length; i++) {
        if (e.target == SVGComponents[i].children[2].children[1]) {
            let filename = document.getElementsByClassName("SVGList")[0].value;
            let showAttrBtn = SVGComponents[i].children[2].children[1];
            let componentTitle = SVGComponents[i].children[0].textContent;
            let componentTitleArray = componentTitle.split(" ");
            let componentName = componentTitleArray[0]; // the shape type
            let componentNum = componentTitleArray[1]-1; // its index

            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/getOtherAttributes',
                data: {
                    filename: filename,
                    shapeType: componentName,
                    componentNum: componentNum,
                },
                success: function (data) {
                    console.log("Loading up the components attributes...");
                    // console.log(data);

                    let attrTable = document.createElement("div");
                    let innerHTML;
                    attrTable.setAttribute("id", "componentAttributes");
                    innerHTML = `
                        <h3>`+ SVGComponents[i].children[0].innerHTML +` - Attributes</h3>
                        <table class="table tableFormat" id="componentAttributesTable">
                            <tr>
                                <th>Attribute Title</th>
                                <th>Attribute Value</th>
                            </tr>`;

                    // fill up the table with dynamic values
                    for (let i = 0; i < data.length; i++) {
                        let array = Object.entries(data[i]);
                        for (let j = 0; j < array.length; j++) {
                            innerHTML += `
                            <tr class="attrData">
                                <td>`+array[j][1]+`</td>`
                            j++
                            innerHTML += `
                                <td>`+array[j][1]+`<br>
                                    <div class="smallBtn attrValueEditBtn">Edit Value</div>
                                </td>
                            </tr>`;
                        }
                    }

                    // close off the table
                    innerHTML += `
                    </table>
                    <div id="addAttributeContainer">
                        <div class="smallBtn" id="addAttribute">Add Attribute</div>
                    </div>`;
                    attrTable.innerHTML = innerHTML;

                    if (showAttrBtn.innerHTML === "Show Attributes") {
                        if (document.getElementById("componentAttributes")) {
                            alert("Please hide current components attributes ")
                        } else {
                            console.log("Showing all attributes");
                            document.getElementById("SvgViewPannel").appendChild(attrTable);
                            showAttrBtn.innerHTML = "Hide Attributes";
                        }
                    } else {
                        console.log("Hiding all attributes");
                        document.getElementById("SvgViewPannel").removeChild(document.getElementById("componentAttributes"));
                        showAttrBtn.innerHTML = "Show Attributes";
                    }
                },
                fail: function(error) {
                    console.log("Failed to load up the components attributes");
                    console.log(error); 
                }
            });
        }
    }
});




document.addEventListener('click', function(e) {
    if (document.getElementById("componentAttributes")) {
        let attrData = document.getElementsByClassName("attrData");
        for (let i = 0; i < attrData.length; i++) {
            // we are currently looping through the attrData's, but how do we know if its in edit mode or not? 
            // We could check the amount of childnodes of attrData[i].children[0]
            if (attrData[i].children[1].children.length === 2 && e.target == attrData[i].children[1].children[1]) { 
                // we are not in edit mode, and click edit, this will happen:
                console.log("Editing the attribute...");
                let textArea = document.createElement("textarea");
                textArea.classList.add("editText");
                textArea.innerHTML = attrData[i].children[1].childNodes[0].textContent;
                let br = document.createElement("br");
                let cancelEdit = document.createElement("div");
                cancelEdit.innerHTML = "Cancel";
                cancelEdit.classList.add("smallBtn");
                cancelEdit.classList.add("attrNameCancelEditBtn");
                let saveEdit = document.createElement("div");
                saveEdit.innerHTML = "Save";
                saveEdit.classList.add("smallBtn");
                saveEdit.classList.add("attrNameSaveEditBtn");
            
                attrData[i].children[1].innerHTML = '';
                attrData[i].children[1].appendChild(textArea);
                attrData[i].children[1].appendChild(br);
                attrData[i].children[1].appendChild(cancelEdit);
                attrData[i].children[1].appendChild(saveEdit);        
            }
            if (attrData[i].children[1].children.length === 4 && e.target == attrData[i].children[1].children[2]) { 
                // we are in edit mode and we click cancel, this will happen:
                console.log("Cancel editing the attribute");

                attrData[i].children[1].innerHTML = attrData[i].children[1].children[0].innerHTML;
                let editBtn = document.createElement("div");
                let br = document.createElement("br");
                editBtn.innerHTML = "Edit Value";
                editBtn.classList.add("smallBtn");
                editBtn.classList.add("attrNameEditBtn");
            
                attrData[i].children[1].appendChild(br);
                attrData[i].children[1].appendChild(editBtn);

            }
            if (attrData[i].children[1].children.length === 4 && e.target == attrData[i].children[1].children[3]) { 
                // we are in edit mode and we click save, this will happen:
                console.log("Save editing the attribute");
                jQuery.ajax({
                    type: 'get',
                    dataType: 'text',
                    url: '/editAttribute',
                    success: function (data) {
                        console.log(data);
                    },
                    fail: function(error) {
                        console.log(error); 
                    }
                });

            }
        }

        
        if (e.target && e.target.id === "addAttribute") {
            console.log("Adding new attribute...");
            // create the new div
            // change button to "cancel" and "save"
            let newAttrForm = document.createElement("table");
            newAttrForm.classList.add("table");
            newAttrForm.classList.add("tableFormat");
            newAttrForm.setAttribute("id", "newAttrForm");
            newAttrForm.innerHTML = `
                <tr>
                    <th>New Attribute Title</th>
                    <th>New Attribute Value</th>
                </tr>
                <tr class="attrData">
                    <td><textarea></textarea></td>
                    <td><textarea></textarea></td>
                </tr>`;

            document.getElementById("addAttributeContainer").innerHTML = `
            <div class="smallBtn" id="cancelAddAttribute">Cancel</div>
            <div class="smallBtn" id="saveAddAttribute">Save</div>`;

            if (!document.getElementById("newAttrForm")) {
                document.getElementById("componentAttributes").appendChild(newAttrForm);
            }
        }

        if (e.target && e.target.id === "cancelAddAttribute") {
            console.log("Cancel adding new attribute");
            document.getElementById("addAttributeContainer").innerHTML = `
            <div class="smallBtn" id="addAttribute">Add Attribute</div>`;

            document.getElementById("componentAttributes").removeChild(document.getElementById("newAttrForm"));
        }

        if (e.target && e.target.id === "saveAddAttribute") {
            console.log("Save adding new attribute");
            jQuery.ajax({
                type: 'get',
                dataType: 'text',
                url: '/addAttribute',
                success: function (data) {
                    console.log(data);
                },
                fail: function(error) {
                    console.log(error); 
                }
            });
        }

    }
});


document.addEventListener('click', function(e) {
    if (e.target && e.target.id === "addRect" && !document.getElementById("addComponentTable")) {
        console.log("Loading table to add rectangle...");
        let additionalFunctionality = document.getElementById("additionalFunctionality");

        let h4 = document.createElement("h4");
        h4.setAttribute("id", "addComponentType");
        h4.innerHTML = "Rectangle";
        additionalFunctionality.insertBefore(h4, document.getElementById("scaleShapesTitle"));

        let rectTable = document.createElement("table");
        rectTable.classList.add("table");
        rectTable.classList.add("tableFormat");
        rectTable.setAttribute("id", "addComponentTable");
        rectTable.innerHTML = `
            <tr>
                <th>X</th>
                <th>Y</th>
                <th>Width</th>
                <th>Height</th>
                <th>Units</th>
            </tr>
            <tr>
                <td><input type="number" class="editTextSmall"></td>
                <td><input type="number" class="editTextSmall"></td>
                <td><input type="number" class="editTextSmall"></td>
                <td><input type="number" class="editTextSmall"></td>
                <td><input type="text" class="editTextSmall"></td>
            </tr>`;
        additionalFunctionality.insertBefore(rectTable, document.getElementById("scaleShapesTitle"));

        let addComponentContainer = document.createElement("div");
        addComponentContainer.setAttribute("id", "addComponentContainer");
        addComponentContainer.innerHTML = `
        <div class="smallBtn" id="cancelAddComponent">Cancel</div>
        <div class="smallBtn" id="saveAddComponent">Save</div>`;
        additionalFunctionality.insertBefore(addComponentContainer, document.getElementById("scaleShapesTitle"));
    }

    if (e.target && e.target.id === "addCirc" && !document.getElementById("addComponentTable")) {
        console.log("Loading table to add circle...");
        let additionalFunctionality = document.getElementById("additionalFunctionality");

        let h4 = document.createElement("h4");
        h4.setAttribute("id", "addComponentType");
        h4.innerHTML = "Circle";
        additionalFunctionality.insertBefore(h4, document.getElementById("scaleShapesTitle"));

        let circTable = document.createElement("table");
        circTable.classList.add("table");
        circTable.classList.add("tableFormat");
        circTable.setAttribute("id", "addComponentTable");
        circTable.innerHTML = `
        <tr>
            <th>CX</th>
            <th>CY</th>
            <th>R</th>
            <th>Units</th>
        </tr>
        <tr>
            <td><input type="number" class="editTextSmall"></td>
            <td><input type="number" class="editTextSmall"></td>
            <td><input type="number" class="editTextSmall"></td>
            <td><input type="text" class="editTextSmall"></td>
        </tr>`;
        additionalFunctionality.insertBefore(circTable, document.getElementById("scaleShapesTitle"));

        let addComponentContainer = document.createElement("div");
        addComponentContainer.setAttribute("id", "addComponentContainer");
        addComponentContainer.innerHTML = `
        <div class="smallBtn" id="cancelAddComponent">Cancel</div>
        <div class="smallBtn" id="saveAddComponent">Save</div>`;
        additionalFunctionality.insertBefore(addComponentContainer, document.getElementById("scaleShapesTitle"));
    }

    if (e.target && e.target.id === "cancelAddComponent" && document.getElementById("addComponentTable")) {
        console.log("Removing table to add component...");
        let additionalFunctionality = document.getElementById("additionalFunctionality");
        additionalFunctionality.removeChild(document.getElementById("addComponentType"));
        additionalFunctionality.removeChild(document.getElementById("addComponentTable"));
        additionalFunctionality.removeChild(document.getElementById("addComponentContainer"));
        // additionalFunctionality.removeChild(document.getElementById("addComponentContainer"));
    }

    if (e.target && e.target.id === "saveAddComponent" && document.getElementById("addComponentTable")) {
        let addComponentTable = document.getElementById("addComponentTable");
        let filename = document.getElementsByClassName("SVGList")[1].value;

        if (filename == "") {
            alert("No file selected");
        }
        if (document.getElementById("addComponentType").innerHTML == "Rectangle" && filename != "") {
            let x = addComponentTable.children[0].children[1].children[0].children[0].value;
            let y = addComponentTable.children[0].children[1].children[1].children[0].value;
            let width = addComponentTable.children[0].children[1].children[2].children[0].value;
            let height = addComponentTable.children[0].children[1].children[3].children[0].value;
            let units = addComponentTable.children[0].children[1].children[4].children[0].value;

            let valid = 1;
            if (x == "" || y == "" || width == "" || height == "") {
                valid = 0;
            }

            if (units.length > 49) {
                alert("Lenght of units cannot be greater then 49 chars")
            } else if (valid == 0) {
                alert("Input is invalid");
            } else {
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'text',       //Data type - we will use JSON for almost everything 
                    url: '/addRect',   //The server endpoint we are connecting to
                    data: {
                        x: x,
                        y: y,
                        width: width,
                        height: height,
                        units: units,
                        filename: filename,
                    },
                    success: function (data) {
                        console.log(data);
                        if (data == "invalid") {
                            alert("data is invalid");
                        } else if (data == "valid") {
                            console.log("Component added successfully");
                            location.reload();
                        }
                    },
                    fail: function(error) {
                        // Non-200 return, do something with error
                        console.log("Component failed to add");
                        console.log(error); 
                    }
                });
            }

        } else if (filename != "") {
            let cx = addComponentTable.children[0].children[1].children[0].children[0].value;
            let cy = addComponentTable.children[0].children[1].children[1].children[0].value;
            let r = addComponentTable.children[0].children[1].children[2].children[0].value;
            let units = addComponentTable.children[0].children[1].children[3].children[0].value;

            let valid = 1;
            if (cx == "" || cy == "" || r == "") {
                valid = 0;
            }

            if (units.length > 49) {
                alert("Lenght of units cannot be greater then 49 chars")
            } else if (valid == 0) {
                alert("Input is invalid");
            } else {
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'text',       //Data type - we will use JSON for almost everything 
                    url: '/addCirc',   //The server endpoint we are connecting to
                    data: {
                        cx: cx,
                        cy: cy,
                        r: r,
                        units: units,
                        filename: filename,
                    },
                    success: function (data) {
                        console.log(data);
                        if (data == "invalid") {
                            alert("data is invalid");
                        } else if (data == "valid") {
                            console.log("Component added successfully");
                            location.reload();
                        }
                    },
                    fail: function(error) {
                        // Non-200 return, do something with error
                        console.log("Component failed to add");
                        console.log(error); 
                    }
                });
            }
        }

    }


});

document.getElementById("saveCreateSVG").addEventListener('click', function(e) {
    // document.getElementById("createSVG").children[0].children[1].children[i].children
    let createSVG = document.getElementById("createSVG");
    let title = createSVG.children[0].children[1].children[0].children[0].value;
    let description = createSVG.children[0].children[1].children[1].children[0].value;
    let filename = createSVG.children[0].children[1].children[2].children[0].value;
    let valid = 1;

    if (title == undefined) {
        title = " ";
    }
    if (description == undefined) {
        description = " ";
    }
    console.log(title.length);
    if (title.length > 255) {
        alert("title length cannot be greater then 255 chars");
        valid = 0;
    }
    if (description.length > 255) {
        alert("description length cannot be greater then 255 chars");
        valid = 0;
    }

    if (valid) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'text',       //Data type - we will use JSON for almost everything 
            url: '/createSVG',   //The server endpoint we are connecting to
            data: {
                title: title,
                description: description,
                filename: filename,
            },
            success: function (data) {
                console.log(data);
                if (data == "not unique") {
                    alert("filename is not unique");
                } else if (data == "invalid") {
                    alert("SVG is invalid");
                } else if (data == "valid") {
                    console.log("SVG created sucessfully");
                    location.reload();
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log("SVG failed to create");
                console.log(error); 
            }
        });
    }
});

document.getElementById("saveScaleSVG").addEventListener('click', function(e) {
    let scaleShape = document.getElementById("scaleShape");
    let shapeType = scaleShape.children[0].children[1].children[0].children[0].value;
    let scaleFactor = scaleShape.children[0].children[1].children[1].children[0].value;
    let filename = document.getElementsByClassName("SVGList")[2].value;

    let valid = 1;
    if (scaleFactor == "") {
        valid = 0;
    }
    if (filename == "") {
        alert("No file selected");
    }

    if (scaleFactor > 0 && valid == 1 && filename != "") {
        console.log("Scaling SVG...");
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'text',       //Data type - we will use JSON for almost everything 
            url: '/scaleShape',   //The server endpoint we are connecting to
            data: {
                shapeType: shapeType,
                scaleFactor: scaleFactor,
                filename: filename,
            },
            success: function (data) {
                console.log(data);
                if (data == "invalid") {
                    alert("SVG is invalid");
                } else if (data == "valid") {
                    console.log("SVG scaled sucessfully");
                    location.reload();
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log("SVG failed to scale");
                console.log(error); 
            }
        });
    } else if (valid == 0) {
        alert("Input is invalid");
    } else {
        alert("Must scale by a factor greater then 0");
    }
});