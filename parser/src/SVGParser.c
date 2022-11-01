/*
Name: Joffre du Preez
Student ID: 1136583

Code that i used:
I used the code from the tree1.c example
The link to the code is http://www.xmlsoft.org/examples/tree1.c
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "../include/SVGParser.h"
// #include "LinkedListAPI.c"

void getSvgDetails (xmlNode * a_node, SVG *svgStruct);
int findStuffInGroups(List* groupList, char* type, float area);
int findStuffInGroupsForPaths(List* groupList, const char* data);
int countAttributesInGroups(List* groupList);
void getPathsInGroups(List* paths, List* groupList);
void getRectanglesInGroups(List* rectangles, List* groupList);
void getCirclesInGroups(List* circles, List* groupList);
void getGroupsInGroups(List* totalGroups, Group* groupStruct);
void getGroupDetails (xmlNode * a_node, Group *groupStruct);
xmlDocPtr svgToTree (const SVG* img);
void groupToTree (Group* groupStruct, xmlNodePtr root_node);
bool validateGroup (Group* groupStruct);
void getLengthsInGroups(List* groupList, int *numRect, int *numCirc, int *numPaths, int *numGroups);
char* fileSVGtoJSON(const char* fileName, const char* schemaFile);
char* svgViewPannel(const char* fileName);
char* getOtherAttributes(const char* fileName, const char* componentName, int componentNum);
int createSVGfromJSON (const char* svgString, const char* filename);
int JSONAddComponent(const char* shapeString, const char* filename, int type);
int scaleShapes(const char* filename, const char* shapeType, float scaleFactor);

SVG* createSVG(const char* fileName) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    LIBXML_TEST_VERSION
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        return NULL;
    }
    
    // get the root element and initilaize the lists
    root_element = xmlDocGetRootElement(doc);
    SVG *svgStruct = (SVG*)malloc(sizeof(SVG));
    List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    svgStruct->otherAttributes = otherAttributes;

    List* rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    svgStruct->rectangles = rectangles;

    List* circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    svgStruct->circles = circles;

    List* paths = initializeList(&pathToString, &deletePath, &comparePaths);
    svgStruct->paths = paths;

    List* groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    svgStruct->groups = groups;

    // set title and description to "" to get rid of valgrind errors
    strcpy(svgStruct->title, "");
    strcpy(svgStruct->description, "");
    // call getSvgDetails to recursivly go through the svg and populate svgStruct
    getSvgDetails(root_element, svgStruct);
    // get the namespace
	if (strlen((char *)root_element->ns->href) > 255) {
		char* temp = (char*)malloc(500);
		strcpy(temp, ((char *)root_element->ns->href));
		temp[256] = '\0';
		strcpy(svgStruct->namespace, temp);
		free(temp);
	} else {
		strcpy(svgStruct->namespace, (char *)(root_element->ns->href));	
	}

    xmlFreeDoc(doc);

    return svgStruct;
}





char* SVGToString(const SVG* img) {
	char* svgStr;
	char* groupStr;
	char* attrStr;
	char* rectStr;
	char* circleStr;
	char* pathStr;
	char* finalStr;
    int length = 0;
	
	if (img == NULL){
		return NULL;
	}
	
	svgStr = (char*)malloc(sizeof(char)*(256*3));

	sprintf(svgStr, "namespace = %s, title = %s, description = %s\n", img->namespace, img->title, img->description);
    length += sizeof(char)*(strlen(svgStr) + 1);

    attrStr = attributeToString((void*)img->otherAttributes->head);
    if (attrStr != NULL) {
        length += sizeof(char)*(strlen(attrStr) + 1);
        length += 1; // for the newline character
    }

    rectStr = rectangleToString((void*)img->rectangles->head);
    if (rectStr != NULL) {
        length += sizeof(char)*(strlen(rectStr) + 1);
        length += 1; // for the newline character
    }

    circleStr = circleToString((void*)img->circles->head);
    if (circleStr != NULL) {
        length += sizeof(char)*(strlen(circleStr) + 1);
        length += 1; // for the newline character
    }

    pathStr = pathToString((void*)img->paths->head);
    if (pathStr != NULL) {
        length += sizeof(char)*(strlen(pathStr) + 1);
        length += 1; // for the newline character
    }

    groupStr = groupToString((void*)img->groups->head);
    if (groupStr != NULL) {
        length += sizeof(char)*(strlen(groupStr) + 1);
        length += 1; // for the newline character
    }

    finalStr = (char *) malloc(length);

    strcpy(finalStr, svgStr);
    strcat(finalStr, "\n");
    free(svgStr);

    if (attrStr != NULL) {
        strcat(finalStr, attrStr);
        strcat(finalStr, "\n");
        free(attrStr);
    }

    if (rectStr != NULL) {
        strcat(finalStr, rectStr);
        strcat(finalStr, "\n");
        free(rectStr);
    }

    if (circleStr != NULL) {
        strcat(finalStr, circleStr);
        strcat(finalStr, "\n");
        free(circleStr);
    }

    if (pathStr != NULL) {
        strcat(finalStr, pathStr);
        strcat(finalStr, "\n");
        free(pathStr);
    }

    if (groupStr != NULL) {
        strcat(finalStr, groupStr);
        strcat(finalStr, "\n");
        free(groupStr);
    }

	return finalStr;
}





void deleteSVG(SVG* img) {
    if (img != NULL) {
        freeList(img->otherAttributes);
        freeList(img->rectangles);
        freeList(img->circles);
        freeList(img->paths);
        freeList(img->groups);
        free(img);
    }
}





// Function that returns a list of all rectangles in the struct.  
List* getRects(const SVG* img) {
    List* rectangles;
    if (img == NULL) {
        rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    } else {
        rectangles = img->rectangles;
        getRectanglesInGroups(rectangles, img->groups);   
    }

    return rectangles;
}






// Function that returns a list of all circles in the struct.  
List* getCircles(const SVG* img) {
    List* circles;

    if (img == NULL) {
        circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    } else {
        circles = img->circles;
        getCirclesInGroups(circles, img->groups);
    }

    return circles;
}






// Function that returns a list of all groups in the struct.  
List* getGroups(const SVG* img) {
    List* totalGroups;
    if (img == NULL) {
        totalGroups = initializeList(&groupToString, &deleteGroup, &compareGroups);
        return totalGroups;
    }

    totalGroups = initializeList(&groupToString, &deleteGroup, &compareGroups);

    List* groups = img->groups;
    Node* groupHead = groups->head;
    Node* savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;
        insertBack(totalGroups, (void*)groupData);
        getGroupsInGroups(totalGroups, groupData);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;

    return totalGroups;
}






// Function that returns a list of all paths in the struct.  
List* getPaths(const SVG* img) {
    List* paths;

    if (img == NULL) {
        paths = initializeList(&pathToString, &deletePath, &comparePaths);
    } else {
        paths = img->paths;
        getPathsInGroups(paths, img->groups);
    }



    return paths;
}





// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(const SVG* img, float area) {
    if (img == NULL) {
        return 0;
    }

    int numRects = 0;
    int intArea;

    Node* rectListHead;
    Rectangle* tmpRectangle;

    rectListHead = img->rectangles->head;
    // calculate the number of rectangle with the specified area in the non-nested rectangles
    while (rectListHead != NULL) {
        tmpRectangle = (Rectangle*)rectListHead->data;

        intArea = ceil(tmpRectangle->width*tmpRectangle->height);

        if (intArea == ceil(area)) {
            numRects++;
        }
        rectListHead = rectListHead->next;
    }

    numRects += findStuffInGroups(img->groups, "rectangle", area);

    return numRects;
}





// Function that returns the number of all circles with the specified area
int numCirclesWithArea(const SVG* img, float area) {
    if (img == NULL) {
        return 0;
    }

    int numCircles = 0;
    int intArea;

    Node* circleListHead;
    Circle* tmpCircle;

    circleListHead = img->circles->head;
    // calculate the number of rectangle with the specified area in the non-nested rectangles
    while (circleListHead != NULL) {
        tmpCircle = (Circle*)circleListHead->data;

        intArea = ceil((3.14159 * (tmpCircle->r * tmpCircle->r)));

        if (intArea == ceil(area)) {
            numCircles++;
        }
        circleListHead = circleListHead->next;
    }

    numCircles += findStuffInGroups(img->groups, "circle", area);

    return numCircles;
}





// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(const SVG* img, const char* data) {
    if (img == NULL) {
        return 0;
    }

    int numPaths = 0;

    Node* pathListHead;
    Path* tmpPath;

    pathListHead = img->paths->head;
    // calculate the number of rectangle with the specified area in the non-nested rectangles
    while (pathListHead != NULL) {
        tmpPath = (Path*)pathListHead->data;

        if (strcmp(tmpPath->data, data) == 0) {
            numPaths++;
        }

        pathListHead = pathListHead->next;
    }

    numPaths += findStuffInGroupsForPaths(img->groups, data);

    return numPaths;
}





// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(const SVG* img, int len) {
    if (img == NULL) {
        return 0;
    }

    Node * groupHead;
    Group* tmpGroup;
    int length;
    int numGroups = 0;

    groupHead = img->groups->head;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        length = tmpGroup->rectangles->length + tmpGroup->circles->length + tmpGroup->paths->length + tmpGroup->groups->length;

        if (length == len) {
            numGroups++;
        }

        groupHead = groupHead->next;
    }

    return numGroups;
}




int numAttr(const SVG* img) {
    if (img == NULL) {
        return 0;
    }

    int numAttributes = 0;

    // rectangles
    Node* rectHead = img->rectangles->head;
    while (rectHead != NULL) {
        Rectangle* rectData = (Rectangle*)rectHead->data;
        numAttributes += rectData->otherAttributes->length;
        rectHead = rectHead->next;
    }
    // circles
    Node* circleHead = img->circles->head;
    while (circleHead != NULL) {
        Circle* circleData = (Circle*)circleHead->data;
        numAttributes += circleData->otherAttributes->length;
        circleHead = circleHead->next;
    }
    // paths
    Node* pathHead = img->paths->head;
    while (pathHead != NULL) {
        Path* pathData = (Path*)pathHead->data;
        numAttributes += pathData->otherAttributes->length;
        pathHead = pathHead->next;
    }

    numAttributes += img->otherAttributes->length;
    
    numAttributes += countAttributesInGroups(img->groups);

    return numAttributes;

}





bool validateSVG(const SVG* img, const char* schemaFile) {
    if (img == NULL) {
        return false;
    }
    if (schemaFile == NULL) {
        return false;
    }

    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    // char *XMLFileName = "validTest.svg";
    char *XMLFileName = "validation.svg";
    char *XSDFileName = (char*)malloc(strlen(schemaFile) + 1);
    strcpy(XSDFileName, schemaFile);
    int ret = 0;

    xmlDocPtr testFile = svgToTree(img);

    xmlSaveFormatFileEnc(XMLFileName, testFile, "UTF-8", 1);
    xmlFreeDoc(testFile);

    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(XSDFileName);
    free(XSDFileName);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    //xmlSchemaDump(stdout, schema); //To print schema dump


    doc = xmlReadFile(XMLFileName, NULL, 0);
    // doc = svgToTree(img);

    if (doc == NULL){
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    } else{
        xmlSchemaValidCtxtPtr ctxt;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        // if (ret == 0) {
        //     printf("%s validates\n", XMLFileName);
        // } else if (ret > 0) {
        //     printf("%s fails to validate\n", XMLFileName);
        // } else{
        //     printf("%s validation generated an internal error\n", XMLFileName);
        // }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
    }

    // free the resource
    if(schema != NULL) {
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    // xmlCleanupParser();
    xmlMemoryDump();

    if (ret != 0) {
        return false;
    }





    if (strlen(img->namespace) == 0) {
        return false;
    }
    if (img->rectangles == NULL || img->circles == NULL || img->paths == NULL || img->groups == NULL || img->otherAttributes == NULL) {
        return false;
    }

    List* otherAttributes = img->otherAttributes;
    Node* attributeHead = otherAttributes->head;
    Node* savedAttributeHead = otherAttributes->head;
    while (attributeHead != NULL) {
        Attribute* attributedata = (Attribute*)attributeHead->data;
        if (attributedata->name == NULL) {
            return false;
        }
        attributeHead = attributeHead->next;
    }
    attributeHead = savedAttributeHead;

    // go through the rectangles
    List* rectangles = img->rectangles;
    Node* rectangleHead = rectangles->head;
    Node* savedHead = rectangles->head;
    while (rectangleHead != NULL) {
        Rectangle* rectangleData = (Rectangle*)rectangleHead->data;

        if (rectangleData->width < 0 || rectangleData->height < 0) {
            return false;
        }
        if (rectangleData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = rectangleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        rectangleHead = rectangleHead->next;
    }
    rectangleHead = savedHead;


    // go through the circles
    List* circles = img->circles;
    Node* circleHead = circles->head;
    savedHead = circles->head;
    while (circleHead != NULL) {
        Circle* circleData = (Circle*)circleHead->data;

        if (circleData->r < 0) {
            return false;
        }
        if (circleData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = circleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        circleHead = circleHead->next;
    }
    circleHead = savedHead;


    // go through the paths
    List* paths = img->paths;
    Node* pathHead = paths->head;
    savedHead = paths->head;
    while (pathHead != NULL) {
        Path* pathData = (Path*)pathHead->data;

        if (pathData->data == NULL) {
            return false;
        }
        if (pathData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = pathData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        pathHead = pathHead->next;
    }
    pathHead = savedHead;


    // go through the groups
    List* groups = img->groups;
    Node* groupHead = groups->head;
    savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;

        if (groupData->rectangles == NULL || groupData->circles == NULL || groupData->paths == NULL || groupData->groups == NULL || groupData->otherAttributes == NULL) {
            return false;
        }

        // add the otherAttributes of the groups
        List* otherAttributes = groupData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        if (!validateGroup(groupData)) {
            return false;
        }

        groupHead = groupHead->next;
    }
    groupHead = savedHead;

    return true;
}





SVG* createValidSVG(const char* fileName, const char* schemaFile) {
    xmlDocPtr doc = NULL;
    xmlNode *root_element = NULL;

    LIBXML_TEST_VERSION
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        return NULL;
    }
    if (schemaFile == NULL) {
        return NULL;
    }

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    int ret = 0;

    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(schemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    if (doc == NULL){
        fprintf(stderr, "Could not parse %s\n", fileName);
    } else {
        xmlSchemaValidCtxtPtr ctxt;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        // if (ret == 0) {
        //     printf("%s validates\n", fileName);
        // } else if (ret > 0) {
        //     printf("%s fails to validate\n", fileName);
        // } else {
        //     printf("%s validation generated an internal error\n", fileName);
        // }
        xmlSchemaFreeValidCtxt(ctxt);
    }

    // free the resource
    if(schema != NULL) {
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    // xmlCleanupParser();
    xmlMemoryDump();

    if (ret != 0) {
        return NULL;
    }
    
    // get the root element and initilaize the lists
    root_element = xmlDocGetRootElement(doc);
    SVG *svgStruct = (SVG*)malloc(sizeof(SVG));
    List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    svgStruct->otherAttributes = otherAttributes;

    List* rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    svgStruct->rectangles = rectangles;

    List* circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    svgStruct->circles = circles;

    List* paths = initializeList(&pathToString, &deletePath, &comparePaths);
    svgStruct->paths = paths;

    List* groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    svgStruct->groups = groups;

    // set title and description to "" to get rid of valgrind errors
    strcpy(svgStruct->title, "");
    strcpy(svgStruct->description, "");
    // call getSvgDetails to recursivly go through the svg and populate svgStruct
    getSvgDetails(root_element, svgStruct);
    // get the namespace
	if (strlen((char *)root_element->ns->href) > 255) {
		char* temp = (char*)malloc(500);
		strcpy(temp, ((char *)root_element->ns->href));
		temp[256] = '\0';
		strcpy(svgStruct->namespace, temp);
		free(temp);
	} else {
		strcpy(svgStruct->namespace, (char *)(root_element->ns->href));	
	}

    xmlFreeDoc(doc);

    return svgStruct;
}





bool writeSVG(const SVG* img, const char* fileName) {
    if (img == NULL) {
        return false;
    }
    if (fileName == NULL) {
        return false;
    }

    xmlDocPtr doc = svgToTree(img);

    xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return true;
}





bool setAttribute(SVG* img, elementType elemType, int elemIndex, Attribute* newAttribute) {
    int index, boolValue, attributeFound, shapeFound;

    boolValue = 1;
    if (img == NULL || newAttribute == NULL) {
        return false;
    }
    if (elemType < 0 || elemType > 4) {
        return false;
    }
    if (newAttribute->name == NULL || newAttribute->value == NULL) {
        return false;
    }

    if (elemType == 0) {
        index = attributeFound = 0;
        if (strcmp(newAttribute->name, "xmlns") == 0) {
            strcpy(img->namespace, newAttribute->value);
            attributeFound = 1;
        }

        List* otherAttributes = img->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (strcmp(newAttribute->name, attributedata->name) == 0) {
                strcpy(attributedata->value, newAttribute->value);
                attributeFound = 1;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        if (attributeFound == 0) {
            Attribute* newAttr;
            newAttr = malloc(sizeof(Attribute) + strlen(newAttribute->value)+1);
            newAttr->name = (char*)malloc(strlen(newAttribute->name)+1);
            strcpy(newAttr->name, newAttribute->name);
            strcpy(newAttr->value, newAttribute->value);

            insertBack(img->otherAttributes, (void*)newAttr);
            free(newAttribute->name);
            free(newAttribute);
        }

    } else if (elemType == 1) {
        shapeFound = index = 0;

        List* circles = img->circles;
        Node* circleHead = circles->head;
        Node* savedHead = circles->head;
        while (circleHead != NULL) {
            if (index == elemIndex) {
                shapeFound = 1;
                attributeFound = 0;
                Circle* circleData = (Circle*)circleHead->data;

                if (strcmp(newAttribute->name, "cx") == 0) {
                    attributeFound = 1;
                    circleData->cx = atof(newAttribute->value);
                } else if (strcmp(newAttribute->name, "cy") == 0) {
                    attributeFound = 1;
                    circleData->cy = atof(newAttribute->value);
                } else if (strcmp(newAttribute->name, "r") == 0) {
                    attributeFound = 1;
                    circleData->r = atof(newAttribute->value);
                }

                // add the otherAttributes of the circles
                List* otherAttributes = circleData->otherAttributes;
                Node* attributeHead = otherAttributes->head;
                Node* savedAttributeHead = otherAttributes->head;
                while (attributeHead != NULL) {
                    Attribute* attributedata = (Attribute*)attributeHead->data;
                    if (strcmp(newAttribute->name, attributedata->name) == 0) {
                        strcpy(attributedata->value, newAttribute->value);
                        attributeFound = 1;
                    }
                    attributeHead = attributeHead->next;
                }
                attributeHead = savedAttributeHead;

                if (attributeFound == 0) {
                    Attribute* newAttr;
                    newAttr = malloc(sizeof(Attribute) + strlen(newAttribute->value)+1);
                    newAttr->name = (char*)malloc(strlen(newAttribute->name)+1);
                    strcpy(newAttr->name, newAttribute->name);
                    strcpy(newAttr->value, newAttribute->value);

                    insertBack(circleData->otherAttributes, (void*)newAttr);
                    free(newAttribute->name);
                    free(newAttribute);
                }
            }
            circleHead = circleHead->next;
            index++;
        }
        circleHead = savedHead;
        boolValue = shapeFound;

    } else if (elemType == 2) {
        shapeFound = index = 0;

        List* rectangles = img->rectangles;
        Node* rectangleHead = rectangles->head;
        Node* savedHead = rectangles->head;
        while (rectangleHead != NULL) {
            if (index == elemIndex) {
                shapeFound = 1;
                attributeFound = 0;
                Rectangle* rectangleData = (Rectangle*)rectangleHead->data;

                if (strcmp(newAttribute->name, "x") == 0) {
                    attributeFound = 1;
                    rectangleData->x = atof(newAttribute->value);
                } else if (strcmp(newAttribute->name, "y") == 0) {
                    attributeFound = 1;
                    rectangleData->y = atof(newAttribute->value);
                } else if (strcmp(newAttribute->name, "width") == 0) {
                    attributeFound = 1;
                    rectangleData->width = atof(newAttribute->value);
                } else if (strcmp(newAttribute->name, "height") == 0) {
                    attributeFound = 1;
                    rectangleData->height = atof(newAttribute->value);
                }

                // add the otherAttributes of the rectangles
                List* otherAttributes = rectangleData->otherAttributes;
                Node* attributeHead = otherAttributes->head;
                Node* savedAttributeHead = otherAttributes->head;
                while (attributeHead != NULL) {
                    Attribute* attributedata = (Attribute*)attributeHead->data;
                    if (strcmp(newAttribute->name, attributedata->name) == 0) {
                        strcpy(attributedata->value, newAttribute->value);
                        attributeFound = 1;
                    }
                    attributeHead = attributeHead->next;
                }
                attributeHead = savedAttributeHead;

                if (attributeFound == 0) {
                    Attribute* newAttr;
                    newAttr = malloc(sizeof(Attribute) + strlen(newAttribute->value)+1);
                    newAttr->name = (char*)malloc(strlen(newAttribute->name)+1);
                    strcpy(newAttr->name, newAttribute->name);
                    strcpy(newAttr->value, newAttribute->value);

                    insertBack(rectangleData->otherAttributes, (void*)newAttr);
                    free(newAttribute->name);
                    free(newAttribute);
                }
            }
            rectangleHead = rectangleHead->next;
            index++;
        }
        rectangleHead = savedHead;
        boolValue = shapeFound;
        
    } else if (elemType == 3) {
        shapeFound = index = 0;

        List* paths = img->paths;
        Node* pathHead = paths->head;
        Node* savedHead = paths->head;
        while (pathHead != NULL) {
            if (index == elemIndex) {
                shapeFound = 1;
                attributeFound = 0;
                Path* pathData = (Path*)pathHead->data;

                if (strcmp(newAttribute->name, "d") == 0) {
                    attributeFound = 1;
                    strcpy(pathData->data, newAttribute->value);
                }

                // add the otherAttributes of the paths
                List* otherAttributes = pathData->otherAttributes;
                Node* attributeHead = otherAttributes->head;
                Node* savedAttributeHead = otherAttributes->head;
                while (attributeHead != NULL) {
                    Attribute* attributedata = (Attribute*)attributeHead->data;
                    if (strcmp(newAttribute->name, attributedata->name) == 0) {
                        strcpy(attributedata->value, newAttribute->value);
                        attributeFound = 1;
                    }
                    attributeHead = attributeHead->next;
                }
                attributeHead = savedAttributeHead;

                if (attributeFound == 0) {
                    Attribute* newAttr;
                    newAttr = malloc(sizeof(Attribute) + strlen(newAttribute->value)+1);
                    newAttr->name = (char*)malloc(strlen(newAttribute->name)+1);
                    strcpy(newAttr->name, newAttribute->name);
                    strcpy(newAttr->value, newAttribute->value);

                    insertBack(pathData->otherAttributes, (void*)newAttr);
                    free(newAttribute->name);
                    free(newAttribute);
                }
            }
            pathHead = pathHead->next;
            index++;
        }
        pathHead = savedHead;
        boolValue = shapeFound;

    } else if (elemType == 4) {
        shapeFound = index = 0;

        List* groups = img->groups;
        Node* groupHead = groups->head;
        Node* savedHead = groups->head;
        while (groupHead != NULL) {
            if (index == elemIndex) {
                shapeFound = 1;
                attributeFound = 0;
                Group* groupData = (Group*)groupHead->data;

                // add the otherAttributes of the groups
                List* otherAttributes = groupData->otherAttributes;
                Node* attributeHead = otherAttributes->head;
                Node* savedAttributeHead = otherAttributes->head;
                while (attributeHead != NULL) {
                    Attribute* attributedata = (Attribute*)attributeHead->data;
                    if (strcmp(newAttribute->name, attributedata->name) == 0) {
                        strcpy(attributedata->value, newAttribute->value);
                        attributeFound = 1;
                    }
                    attributeHead = attributeHead->next;
                }
                attributeHead = savedAttributeHead;

                if (attributeFound == 0) {
                    Attribute* newAttr;
                    newAttr = malloc(sizeof(Attribute) + strlen(newAttribute->value)+1);
                    newAttr->name = (char*)malloc(strlen(newAttribute->name)+1);
                    strcpy(newAttr->name, newAttribute->name);
                    strcpy(newAttr->value, newAttribute->value);

                    insertBack(groupData->otherAttributes, (void*)newAttr);
                    free(newAttribute->name);
                    free(newAttribute);
                }
            }
            groupHead = groupHead->next;
            index++;
        }
        groupHead = savedHead;
        boolValue = shapeFound;
    }

    if (boolValue) {
        return true;
    } else {
        return false;
    }
}





void addComponent(SVG* img, elementType type, void* newElement) {
    if (img == NULL) {
        return;
    }
    if (newElement == NULL) {
        return;
    }

    if (type == 1) {
        Circle* tmpCircle = (Circle*)newElement;
        insertBack(img->circles, (void*)tmpCircle);
    } else if (type == 2) {
        Rectangle* tmpRectangle = (Rectangle*)newElement;
        insertBack(img->rectangles, (void*)tmpRectangle);
    } else if (type == 3) {
        Path* tmpPath = (Path*)newElement;
        insertBack(img->paths, (void*)tmpPath);
    }
}





char* attrToJSON(const Attribute *a) {
    char* jsonAttr = NULL;

    if (a == NULL) {
        jsonAttr = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonAttr, "{}");
    } else {
        jsonAttr = (char*)malloc(5000);
        sprintf(jsonAttr, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
    }

    return jsonAttr;
}





char* circleToJSON(const Circle *c) {
    char* jsonCircle;

    if (c == NULL) {
        jsonCircle = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonCircle, "{}");
    } else {
        int numAttr = c->otherAttributes->length;
        jsonCircle = (char*)malloc(5000);
        sprintf(jsonCircle, "{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", c->cx, c->cy, c->r, numAttr, c->units);
    }

    return jsonCircle;
}





char* rectToJSON(const Rectangle *r) {
    char* jsonRect;

    if (r == NULL) {
        jsonRect = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonRect, "{}");
    } else {
        int numAttr = r->otherAttributes->length;
        jsonRect = (char*)malloc(5000);
        sprintf(jsonRect, "{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", r->x, r->y, r->width, r->height, numAttr, r->units);
    }

    return jsonRect;
}





char* pathToJSON(const Path *p) {
    char* jsonPath;
    char* data = NULL;

    if (p == NULL) {
        jsonPath = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonPath, "{}");
    } else {
        int numAttr = p->otherAttributes->length;
        data = (char*)malloc(strlen(p->data) + 100);
        sprintf(data, "%s", p->data);
        data[64] = '\0';

        jsonPath = (char*)malloc(5000 + strlen(data));
        sprintf(jsonPath, "{\"d\":\"%s\",\"numAttr\":%d}", data, numAttr);
        free(data);
    }

    return jsonPath;
}





char* groupToJSON(const Group *g) {
    char* jsonGroup;

    if (g == NULL) {
        jsonGroup = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonGroup, "{}");
    } else {
        int numAttr = g->otherAttributes->length;
        int childCount = g->rectangles->length + g->circles->length + g->paths->length + g->groups->length;
        jsonGroup = (char*)malloc(5000);
        sprintf(jsonGroup, "{\"children\":%d,\"numAttr\":%d}", childCount, numAttr);
    }

    return jsonGroup;
}





char* attrListToJSON(const List *list) {
    char* listString;
    char* tempString;

    if (list == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }
    Node* listHead = list->head;
    if (listHead == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }

    listString = (char*)malloc(strlen("[")+1);
    strcpy(listString, "[");
    while(listHead != NULL) {
        Attribute* structData = (Attribute*)listHead->data;
        tempString = attrToJSON(structData);
        listString = (char*)realloc(listString, (strlen(listString) + strlen(tempString) + 3));
        strcat(listString, tempString);
        free(tempString);
        strcat(listString, ",");
        listHead = listHead->next;
    }
    listString = (char*)realloc(listString, (strlen(listString) + strlen("]") + 1));
    listString[strlen(listString)-1] = ']'; // recplace the last comma with ]

    return listString;
}





char* circListToJSON(const List *list) {
    char* listString;
    char* tempString;

    if (list == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }
    Node* listHead = list->head;
    if (listHead == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }

    listString = (char*)malloc(strlen("[")+1);
    strcpy(listString, "[");
    while(listHead != NULL) {
        Circle* structData = (Circle*)listHead->data;
        tempString = circleToJSON(structData);
        listString = (char*)realloc(listString, (strlen(listString) + strlen(tempString) + 3));
        strcat(listString, tempString);
        free(tempString);
        strcat(listString, ",");
        listHead = listHead->next;
    }
    listString = (char*)realloc(listString, (strlen(listString) + strlen("]") + 1));
    listString[strlen(listString)-1] = ']'; // recplace the last comma with ]

    return listString;
}





char* rectListToJSON(const List *list) {
    char* listString;
    char* tempString;

    if (list == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }
    Node* listHead = list->head;
    if (listHead == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }

    listString = (char*)malloc(strlen("[")+1);
    strcpy(listString, "[");
    while(listHead != NULL) {
        Rectangle* structData = (Rectangle*)listHead->data;
        tempString = rectToJSON(structData);
        listString = (char*)realloc(listString, (strlen(listString) + strlen(tempString) + 3));
        strcat(listString, tempString);
        free(tempString);
        strcat(listString, ",");
        listHead = listHead->next;
    }
    listString = (char*)realloc(listString, (strlen(listString) + strlen("]") + 1));
    listString[strlen(listString)-1] = ']'; // recplace the last comma with ]

    return listString;
}





char* pathListToJSON(const List *list) {
    char* listString;
    char* tempString;

    if (list == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }
    Node* listHead = list->head;
    if (listHead == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }

    listString = (char*)malloc(strlen("[")+1);
    strcpy(listString, "[");
    while(listHead != NULL) {
        Path* structData = (Path*)listHead->data;
        tempString = pathToJSON(structData);
        listString = (char*)realloc(listString, (strlen(listString) + strlen(tempString) + 3));
        strcat(listString, tempString);
        free(tempString);
        strcat(listString, ",");
        listHead = listHead->next;
    }
    listString = (char*)realloc(listString, (strlen(listString) + strlen("]") + 1));
    listString[strlen(listString)-1] = ']'; // recplace the last comma with ]

    return listString;
}





char* groupListToJSON(const List *list) {
    char* listString;
    char* tempString;

    if (list == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }
    Node* listHead = list->head;
    if (listHead == NULL) {
        listString = (char*)malloc(strlen("[]")+1);
        strcpy(listString, "[]");
        return listString;
    }

    listString = (char*)malloc(strlen("[")+1);
    strcpy(listString, "[");
    while(listHead != NULL) {
        Group* structData = (Group*)listHead->data;
        tempString = groupToJSON(structData);
        listString = (char*)realloc(listString, (strlen(listString) + strlen(tempString) + 3));
        strcat(listString, tempString);
        free(tempString);
        strcat(listString, ",");
        listHead = listHead->next;
    }
    listString = (char*)realloc(listString, (strlen(listString) + strlen("]") + 1));
    listString[strlen(listString)-1] = ']'; // recplace the last comma with ]

    return listString;
}





char* SVGtoJSON(const SVG* img) {
    char* jsonSVG;

    if (img == NULL) {
        jsonSVG = (char*)malloc(sizeof("{}")+1);
        strcpy(jsonSVG, "{}");
    } else {
        int numRect, numCirc, numPaths, numGroups;
        numRect = numCirc = numPaths = numGroups = 0;

        numRect += img->rectangles->length;
        numCirc += img->circles->length;
        numPaths += img->paths->length;
        numGroups += img->groups->length;


        getLengthsInGroups(img->groups, &numRect, &numCirc, &numPaths, &numGroups);

        jsonSVG = (char*)malloc(5000);
        sprintf(jsonSVG, "{\"numRect\":%d,\"numCirc\":%d,\"numPaths\":%d,\"numGroups\":%d}", numRect, numCirc, numPaths, numGroups);
    }

    return jsonSVG;
}





SVG* JSONtoSVG(const char* svgString) {
    if (svgString == NULL) {
        return NULL;
    }

    char* idk;
    idk = (char*)malloc(strlen(svgString) + 5);
    strcpy(idk, svgString);

    SVG *svgStruct = (SVG*)malloc(sizeof(SVG));
    List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    svgStruct->otherAttributes = otherAttributes;

    List* rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    svgStruct->rectangles = rectangles;

    List* circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    svgStruct->circles = circles;

    List* paths = initializeList(&pathToString, &deletePath, &comparePaths);
    svgStruct->paths = paths;

    List* groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    svgStruct->groups = groups;

    char* temp;
    temp = strtok(idk, "\""); // title
    temp = strtok(NULL, "\""); // :
    temp = strtok(NULL, "\""); // title value i need
    temp = strtok(NULL, "\""); // title value i need
    // printf("title = %s\n", temp);
    strcpy(svgStruct->title, temp);
    temp = strtok(NULL, "\""); // ,
    temp = strtok(NULL, "\""); // desc
    temp = strtok(NULL, "\""); // :
    temp = strtok(NULL, "\""); // desc value i need
    // printf("desc = %s\n", temp);
    strcpy(svgStruct->description, temp);
    strcpy(svgStruct->namespace, "http://www.w3.org/2000/svg");
    free(idk);
    // printf("%s, %s, %s", svgStruct->title, svgStruct->description, svgStruct->namespace);

    return svgStruct;
}





Rectangle* JSONtoRect(const char* svgString) {
    if (svgString == NULL) {
        return NULL;
    }

    char* idk;
    idk = (char*)malloc(strlen(svgString) + 5);
    strcpy(idk, svgString);

    Rectangle* rect = (Rectangle*)malloc(sizeof(Rectangle));
    List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    rect->otherAttributes = otherAttributes;

    char* temp;
    temp = strtok(idk, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    rect->x = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    rect->y = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    rect->width = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    rect->height = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    strcpy(rect->units, temp);

    if (strcmp(rect->units, "}") == 0) {
        strcpy(rect->units, "");
    }

    // printf("%f, %f, %f, %f, |%s|\n", rect->x, rect->y, rect->width, rect->height, rect->units);

    free(idk);

    return rect;
}





Circle* JSONtoCircle(const char* svgString) {
    if (svgString == NULL) {
        return NULL;
    }

    char* idk;
    idk = (char*)malloc(strlen(svgString) + 5);
    strcpy(idk, svgString);

    Circle* circ = (Circle*)malloc(sizeof(Circle));
    List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    circ->otherAttributes = otherAttributes;

    char* temp;
    temp = strtok(idk, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    circ->cx = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    circ->cy = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    circ->r = atof(temp);
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    temp = strtok(NULL, "\"");
    strcpy(circ->units, temp);

    // printf("%f, %f, %f, %s\n", circ->cx, circ->cy, circ->r, circ->units);

    free(idk);

    return circ;
}





void deleteAttribute(void* data){
	Attribute* tmpAttribute;
	
	if (data == NULL){
		return;
	}
	
	tmpAttribute = (Attribute*)data;
	
	free(tmpAttribute->name);
	free(tmpAttribute);
}

char* attributeToString(void* data){
	char* tmpStr;
	Attribute* tmpAttribute;
	int length = 0;
	
	if (data == NULL){
		return NULL;
	}
	
	Node* savedHead = data;
	Node* tempHead = data;
	while (tempHead != NULL){
		tmpAttribute = (Attribute*)tempHead->data;
		length += (strlen(tmpAttribute->name) + 1);
		length += (strlen(tmpAttribute->value) + 1);
		length += 5; // for the " = " and " ,"
		tempHead = tempHead->next;
	}
	length += 2; // for the "\n"

	tmpStr = (char*)malloc(sizeof(char)*length);
	strcpy(tmpStr, "");

	tempHead = savedHead;
	tmpAttribute = (Attribute*)tempHead->data;
	strcpy(tmpStr, tmpAttribute->name);
	strcat(tmpStr, " = ");
	strcat(tmpStr, tmpAttribute->value);
	strcat(tmpStr, ", ");
	tempHead = tempHead->next;

	while (tempHead != NULL){
		tmpAttribute = (Attribute*)tempHead->data;
		strcat(tmpStr, tmpAttribute->name);
		strcat(tmpStr, " = ");
		strcat(tmpStr, tmpAttribute->value);
		strcat(tmpStr, ", ");
		tempHead = tempHead->next;
	}
	strcat(tmpStr, "\n");
	
	data = savedHead;

	return tmpStr;
}

int compareAttributes(const void *first, const void *second){
    return 0;
}





void deleteGroup(void* data){
	Group* tmpGroup;
	
	if (data == NULL){
		return;
	}
	
	tmpGroup = (Group*)data;
	
    freeList(tmpGroup->otherAttributes);
    freeList(tmpGroup->rectangles);
    freeList(tmpGroup->circles);
    freeList(tmpGroup->paths);
	freeList(tmpGroup->groups);
	free(tmpGroup);
}

char* groupToString(void* data){
	char* groupStr;
	char* attrStr;
	char* rectStr;
	char* circleStr;
	char* pathStr;
	char* loopStr;
	char* finalStr;
    int length = 0;
    int finalLength = 0;
	
	if (data == NULL){
		return NULL;
	}
	Node* groupHead = data;
	Node* tmpGroupHead = data;
	Group* groupData = (Group*)tmpGroupHead->data;

	while (tmpGroupHead != NULL) {
		groupData = (Group*)tmpGroupHead->data;

		attrStr = attributeToString((void*)groupData->otherAttributes->head);
		if (attrStr != NULL) {
			length += sizeof(char)*(strlen(attrStr) + 1);
			length += 1; // for the newline character
			free(attrStr);
		}

		rectStr = rectangleToString((void*)groupData->rectangles->head);
		if (rectStr != NULL) {
			length += sizeof(char)*(strlen(rectStr) + 1);
			length += 1; // for the newline character
			free(rectStr);
		}

		circleStr = circleToString((void*)groupData->circles->head);
		if (circleStr != NULL) {
			length += sizeof(char)*(strlen(circleStr) + 1);
			length += 1; // for the newline character
			free(circleStr);
		}

		pathStr = pathToString((void*)groupData->paths->head);
		if (pathStr != NULL) {
			length += sizeof(char)*(strlen(pathStr) + 1);
			length += 1; // for the newline character
			free(pathStr);
		}

		groupStr = groupToString((void*)groupData->groups->head);
		if (groupStr != NULL) {
			length += sizeof(char)*(strlen(groupStr) + 1);
			length += 1; // for the newline character
			free(groupStr);
		}

		finalLength += length;
		tmpGroupHead = tmpGroupHead->next;
	}

	finalStr = (char *)malloc(finalLength);
	strcpy(finalStr, "");

	tmpGroupHead = groupHead;

	while (tmpGroupHead != NULL) {
		length = 0;
		groupData = (Group*)tmpGroupHead->data;

		attrStr = attributeToString((void*)groupData->otherAttributes->head);
		if (attrStr != NULL) {
			length += sizeof(char)*(strlen(attrStr) + 1);
			length += 1; // for the newline character
		}

		rectStr = rectangleToString((void*)groupData->rectangles->head);
		if (rectStr != NULL) {
			length += sizeof(char)*(strlen(rectStr) + 1);
			length += 1; // for the newline character
		}
		
		circleStr = circleToString((void*)groupData->circles->head);
		if (circleStr != NULL) {
			length += sizeof(char)*(strlen(circleStr) + 1);
			length += 1; // for the newline character
		}

		pathStr = pathToString((void*)groupData->paths->head);
		if (pathStr != NULL) {
			length += sizeof(char)*(strlen(pathStr) + 1);
			length += 1; // for the newline character
		}

		groupStr = groupToString((void*)groupData->groups->head);
		if (groupStr != NULL) {
			length += sizeof(char)*(strlen(groupStr) + 1);
			length += 1; // for the newline character
		}

		loopStr = (char *)malloc(length);
		strcpy(loopStr, "");

		if (attrStr != NULL) {
			if (strlen(loopStr) == 0) {
				strcpy(loopStr, attrStr);
			} else {
				strcat(loopStr, attrStr);
			}
			strcat(loopStr, "\n");
			free(attrStr);
		}

		if (rectStr != NULL) {
			if (strlen(loopStr) == 0) {
				strcpy(loopStr, rectStr);
			} else {
				strcat(loopStr, rectStr);	
			}
			strcat(loopStr, "\n");
			free(rectStr);
		}

		if (circleStr != NULL) {
			if (strlen(loopStr) == 0) {
				strcpy(loopStr, circleStr);
			} else {
				strcat(loopStr, circleStr);	
			}
			strcat(loopStr, "\n");
			free(circleStr);
		}

		if (pathStr != NULL) {
			if (strlen(loopStr) == 0) {
				strcpy(loopStr, pathStr);
			} else {
				strcat(loopStr, pathStr);	
			}
			strcat(loopStr, "\n");
			free(pathStr);
		}

		if (groupStr != NULL) {
			if (strlen(loopStr) == 0) {
				strcpy(loopStr, groupStr);
			} else {
				strcat(loopStr, groupStr);	
			}
			strcat(loopStr, "\n");
			free(groupStr);
		}

		if (strlen(finalStr) == 0) {
			strcpy(finalStr, loopStr);
		} else {
			strcat(finalStr, loopStr);
		}
		if (loopStr != NULL) {
			free(loopStr);
		}
		
		tmpGroupHead = tmpGroupHead->next;
	}

	return finalStr;
}

int compareGroups(const void *first, const void *second){
    return 0;
}





void deleteRectangle(void* data){
	Rectangle* tmpRectangle;
	
	if (data == NULL){
		return;
	}
	
	tmpRectangle = (Rectangle*)data;
	
    freeList(tmpRectangle->otherAttributes);
	free(tmpRectangle);
}

char* rectangleToString(void* data){
	if (data == NULL){
		return NULL;
	}

	Rectangle* tmpRectangle;
	char* finalStr; // this will hold all the data of the entire list
	char* attrStr; // this will hold the otherAttributes of a rectangle struct
	char* tmpStr; // this will hold the x, y, width, height, and units of a rectangle struct
	char* structStr; // this will hold attrStr + tmpStr
	int structLength = 0;
	int listLength = 0;
	
	Node* savedHead = data;
	Node* tempHead = data;


	while (tempHead != NULL){
		tmpRectangle = (Rectangle*)tempHead->data;
		tmpStr = (char*)malloc(500);

        if (strlen(tmpRectangle->units) != 0) {
            sprintf(tmpStr, "x = %f, y = %f, width = %f, height=%f, units=%s\n", tmpRectangle->x, tmpRectangle->y, tmpRectangle->width, tmpRectangle->height, tmpRectangle->units);
        } else {
            sprintf(tmpStr, "x = %f, y = %f, width = %f, height=%f\n", tmpRectangle->x, tmpRectangle->y, tmpRectangle->width, tmpRectangle->height);
        }
        attrStr = attributeToString((void*)tmpRectangle->otherAttributes->head);

		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}
		listLength += structLength;

		free(attrStr);
		free(tmpStr);
		tempHead = tempHead->next;
	}
	
	finalStr = (char*)malloc(listLength);
	strcpy(finalStr, "");
	tempHead = savedHead;

	while (tempHead != NULL){
		tmpRectangle = (Rectangle*)tempHead->data;
		tmpStr = (char*)malloc(500);

        if (strlen(tmpRectangle->units) != 0) {
            sprintf(tmpStr, "x = %f, y = %f, width = %f, height=%f, units=%s\n", tmpRectangle->x, tmpRectangle->y, tmpRectangle->width, tmpRectangle->height, tmpRectangle->units);
        } else {
            sprintf(tmpStr, "x = %f, y = %f, width = %f, height=%f\n", tmpRectangle->x, tmpRectangle->y, tmpRectangle->width, tmpRectangle->height);
        }
        attrStr = attributeToString((void*)tmpRectangle->otherAttributes->head);


		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}

		structStr = (char*)malloc(structLength);
		strcpy(structStr, tmpStr);
		if (attrStr != NULL) {
			strcat(structStr, attrStr);
		}

		if (strlen(finalStr) == 0) {
			strcpy(finalStr, structStr);
		} else {
			strcat(finalStr, structStr);
		}

		free(attrStr);
		free(tmpStr);
		free(structStr);
		tempHead = tempHead->next;
	}

	data = savedHead;
	return finalStr;
}

int compareRectangles(const void *first, const void *second){
    return 0;
}





void deleteCircle(void* data){
	Circle* tmpCircle;
	
	if (data == NULL){
		return;
	}
	
	tmpCircle = (Circle*)data;
	
    freeList(tmpCircle->otherAttributes);
	free(tmpCircle);
}

char* circleToString(void* data){
	if (data == NULL){
		return NULL;
	}

	Circle* tmpCircle;
	char* finalStr; // this will hold all the data of the entire list
	char* attrStr; // this will hold the otherAttributes of a rectangle struct
	char* tmpStr; // this will hold the x, y, width, height, and units of a rectangle struct
	char* structStr; // this will hold attrStr + tmpStr
	int structLength = 0;
	int listLength = 0;
	
	Node* savedHead = data;
	Node* tempHead = data;


	while (tempHead != NULL){
		tmpCircle = (Circle*)tempHead->data;
		tmpStr = (char*)malloc(500);

        if (strlen(tmpCircle->units) != 0) {
            sprintf(tmpStr, "cx = %f, cy = %f, r = %f, units=%s\n", tmpCircle->cx, tmpCircle->cy, tmpCircle->r, tmpCircle->units);
        } else {
            sprintf(tmpStr, "cx = %f, cy = %f, r = %f\n", tmpCircle->cx, tmpCircle->cy, tmpCircle->r);
        }
        attrStr = attributeToString((void*)tmpCircle->otherAttributes->head);

		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}
		listLength += structLength;

		free(attrStr);
		free(tmpStr);
		tempHead = tempHead->next;
	}

	finalStr = (char*)malloc(listLength);
	strcpy(finalStr, "");
	tempHead = savedHead;

	while (tempHead != NULL){
		tmpCircle = (Circle*)tempHead->data;
		tmpStr = (char*)malloc(500);

        if (strlen(tmpCircle->units) != 0) {
            sprintf(tmpStr, "cx = %f, cy = %f, r = %f, units=%s\n", tmpCircle->cx, tmpCircle->cy, tmpCircle->r, tmpCircle->units);
        } else {
            sprintf(tmpStr, "cx = %f, cy = %f, r = %f\n", tmpCircle->cx, tmpCircle->cy, tmpCircle->r);
        }
        attrStr = attributeToString((void*)tmpCircle->otherAttributes->head);

		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}

		structStr = (char*)malloc(structLength);
		strcpy(structStr, tmpStr);
		if (attrStr != NULL) {
			strcat(structStr, attrStr);
		}

		if (strlen(finalStr) == 0) {
			strcpy(finalStr, structStr);
		} else {
			strcat(finalStr, structStr);
		}

		free(attrStr);
		free(tmpStr);
		free(structStr);
		tempHead = tempHead->next;
	}

	data = savedHead;
	return finalStr;
}

int compareCircles(const void *first, const void *second){
    return 0;
}





void deletePath(void* data){
	Path* tmpPath;
	
	if (data == NULL){
		return;
	}
	
	tmpPath = (Path*)data;
	
    freeList(tmpPath->otherAttributes);
	free(tmpPath);
}

char* pathToString(void* data){
	if (data == NULL){
		return NULL;
	}

	Path* tmpPath;
	char* finalStr; // this will hold all the data of the entire list
	char* attrStr; // this will hold the otherAttributes of a rectangle struct
	char* tmpStr; // this will hold the x, y, width, height, and units of a rectangle struct
	char* structStr; // this will hold attrStr + tmpStr
	int structLength = 0;
	int listLength = 0;
	
	Node* savedHead = data;
	Node* tempHead = data;


	while (tempHead != NULL){
		tmpPath = (Path*)tempHead->data;
		tmpStr = (char*)malloc(500);

		sprintf(tmpStr, "data = %s,\n", tmpPath->data);
		attrStr = attributeToString((void*)tmpPath->otherAttributes->head);

		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}
		listLength += structLength;

		free(attrStr);
		free(tmpStr);
		tempHead = tempHead->next;
	}

	finalStr = (char*)malloc(listLength);
	strcpy(finalStr, "");
	tempHead = savedHead;

	while (tempHead != NULL){
		tmpPath = (Path*)tempHead->data;
		tmpStr = (char*)malloc(500);

		sprintf(tmpStr, "data = %s,\n", tmpPath->data);
		attrStr = attributeToString((void*)tmpPath->otherAttributes->head);

		if (attrStr != NULL) {
			structLength = strlen(tmpStr) + strlen(attrStr) + 2;
		} else {
			structLength = strlen(tmpStr) + 1;
		}

		structStr = (char*)malloc(structLength);
		strcpy(structStr, tmpStr);
		if (attrStr != NULL) {
			strcat(structStr, attrStr);
		}

		if (strlen(finalStr) == 0) {
			strcpy(finalStr, structStr);
		} else {
			strcat(finalStr, structStr);
		}

		free(attrStr);
		free(tmpStr);
		free(structStr);
		tempHead = tempHead->next;
	}

	data = savedHead;
	return finalStr;
}

int comparePaths(const void *first, const void *second){
    return 0;
}

// ************************************************************************
// ************************************************************************
// ************************ HELPER FUNCTIONS ******************************
// ************************************************************************
// ************************************************************************

void getSvgDetails (xmlNode * a_node, SVG *svgStruct) {
    xmlNode *cur_node = NULL;
    if (svgStruct == NULL) {
        return;
    }

    // this iterates through each node on the same level (not nested)
    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            // printf("node type: Element, name: %s\n", cur_node->name);
            // if the current element is a title
            if (strcmp((char *)cur_node->name, "title") == 0) {
                if (strlen((char *)cur_node->children->content) > 255) {
                    char* temp = (char *)cur_node->children->content;
                    temp[256] = '\0';
                    strcpy(svgStruct->title, temp);
                } else {
                    strcpy(svgStruct->title, (char *)cur_node->children->content);
                }
            }
            // if the current element is a description
            if (strcmp((char *)cur_node->name, "desc") == 0) {
                if (strlen((char *)cur_node->children->content) > 255) {
                    char* temp = (char *)cur_node->children->content;
                    temp[256] = '\0';
                    strcpy(svgStruct->description, temp);
                } else {
                    strcpy(svgStruct->description, (char *)cur_node->children->content);
                }
            }
            // if the current element is an SVG
            if (strcmp((const char *)cur_node->name, "svg") == 0) {
                xmlAttr *attr;
                // go through the attributes
                for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                    
                    if (attrName != NULL) {
                        tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                        strcpy(tempAttribute->name, attrName);
                    } else {
                        tempAttribute->name = "Name was NULL";
                    }
                    strcpy(tempAttribute->value, cont);
                    insertBack(svgStruct->otherAttributes, (void*)tempAttribute);
                }
                getSvgDetails(cur_node->children, svgStruct);
            }
            // if the current element is a group
            if (strcmp((const char *)cur_node->name, "g") == 0) {
                // create the tmpGroup and initialize its lists
                Group* tmpGroup = (Group*)malloc(sizeof(Group));

                List* rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
                tmpGroup->rectangles = rectangles;

                List* circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
                tmpGroup->circles = circles;

                List* paths = initializeList(&pathToString, &deletePath, &comparePaths);
                tmpGroup->paths = paths;

                List* groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
                tmpGroup->groups = groups;

                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpGroup->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                // go through the attributes
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                    if (attrName != NULL) {
                        tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                        strcpy(tempAttribute->name, attrName);
                    } else {
                        tempAttribute->name = "Name was NULL";
                    }
                    strcpy(tempAttribute->value, cont);
                    insertBack(tmpGroup->otherAttributes, (void*)tempAttribute);
                }
                // this will go though the children of the group
                getGroupDetails(cur_node->children, tmpGroup);

                // add the group to the svg group list after its populated
                insertBack(svgStruct->groups, (void*)tmpGroup);
            }
            // if the current element is a rectangle
            if (strcmp((const char *)cur_node->name, "rect") == 0) {
                Rectangle* tmpRectangle = (Rectangle*)malloc(sizeof(Rectangle));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpRectangle->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                // go through the attributes
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "x") == 0) {
                        // this is where i parse out the units
                        int character;
                        char* number;
                        char* units;
                        int numCount = 0;
                        int unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++) {
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                numCount++;
                            } else {
                                unitsCount++;
                            }
                        }

                        number = (char*)malloc(numCount+1);
                        units = (char*)malloc(unitsCount+1);
                        numCount = unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++){
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                number[numCount] = cont[i];
                                numCount++;
                            } else {
                                units[unitsCount] = cont[i];
                                unitsCount++;
                            }
                        }
                        number[numCount] = '\0';
                        units[unitsCount] = '\0';

                        tmpRectangle->x = atof(number);
                        strcpy(tmpRectangle->units, units);

                        free(number);
                        free(units);
                    }
                    if (strcmp(attrName, "y") == 0) {
                        tmpRectangle->y = atof(cont);
                    }
                    if (strcmp(attrName, "width") == 0) {
                        tmpRectangle->width = atof(cont);
                    }
                    if (strcmp(attrName, "height") == 0) {
                        tmpRectangle->height = atof(cont);
                    }
                    // go through the other attributes
                    if (strcmp(attrName, "x") != 0 && strcmp(attrName, "y") != 0 && strcmp(attrName, "width") != 0 && strcmp(attrName, "height") != 0) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);

                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);

                        insertBack(tmpRectangle->otherAttributes, (void*)tempAttribute);
                    }
                }

                // add the tmpRectangle to the list
                insertBack(svgStruct->rectangles, (void*)tmpRectangle);
            }
            // if the current element is a circle
            if (strcmp((const char *)cur_node->name, "circle") == 0) {
                Circle* tmpCircle = (Circle*)malloc(sizeof(Circle));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpCircle->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "cx") == 0) {
                        // parse out the units
                        int character;
                        char* number;
                        char* units;
                        int numCount = 0;
                        int unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++) {
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                numCount++;
                            } else {
                                unitsCount++;
                            }
                        }

                        number = (char*)malloc(numCount+1);
                        units = (char*)malloc(unitsCount+1);
                        numCount = unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++){
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                number[numCount] = cont[i];
                                numCount++;
                            } else {
                                units[unitsCount] = cont[i];
                                unitsCount++;
                            }
                        }
                        number[numCount] = '\0';
                        units[unitsCount] = '\0';

                        tmpCircle->cx = atof(number);
                        strcpy(tmpCircle->units, units);

                        free(number);
                        free(units);
                    }
                    if (strcmp(attrName, "cy") == 0) {
                        tmpCircle->cy = atof(cont);
                    }
                    if (strcmp(attrName, "r") == 0) {
                        tmpCircle->r = atof(cont);
                    }
                    if (strcmp(attrName, "cx") != 0 && strcmp(attrName, "cy") != 0 && strcmp(attrName, "r")) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);
                        insertBack(tmpCircle->otherAttributes, (void*)tempAttribute);
                    }
                }

                // add the circle to the list
                insertBack(svgStruct->circles, (void*)tmpCircle);
            }
            // if the current element is a path
            if (strcmp((const char *)cur_node->name, "path") == 0) {
                Path* tmpPath = (Path*)malloc(sizeof(Path));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpPath->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "d") == 0) {
                        if (cont != NULL) {
                            tmpPath = (Path*)realloc(tmpPath, (sizeof(Path) + strlen(cont) + 1));
                            strcpy(tmpPath->data, cont);
                        } else {
                            strcpy(tmpPath->data, "Data was NULL");
                        }
                    }
                    if (strcmp(attrName, "d") != 0) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);
                        insertBack(tmpPath->otherAttributes, (void*)tempAttribute);
                    }
                }

                insertBack(svgStruct->paths, (void*)tmpPath);
            }
        }
    }
}






int findStuffInGroups(List* groupList, char* type, float area) {
    Node* groupHead = groupList->head;

    if (groupHead == NULL) {
        return 0;
    }

    Group* tmpGroup;
    int count = 0;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        if (strcmp(type, "rectangle") == 0) {
            int intArea;

            Node* rectListHead;
            Rectangle* tmpRectangle;

            rectListHead = tmpGroup->rectangles->head;
            // calculate the number of rectangle with the specified area in the non-nested rectangles
            while (rectListHead != NULL) {
                tmpRectangle = (Rectangle*)rectListHead->data;

                intArea = ceil(tmpRectangle->width * tmpRectangle->height);

                if (intArea == ceil(area)) {
                    count++;
                }
                rectListHead = rectListHead->next;
            }
            count += findStuffInGroups(tmpGroup->groups, "rectangle", area);
        }
        if (strcmp(type, "circle") == 0) {
            int intArea;

            Node* circleListHead;
            Circle* tmpCircle;

            circleListHead = tmpGroup->circles->head;
            // calculate the number of rectangle with the specified area in the non-nested rectangles
            while (circleListHead != NULL) {
                tmpCircle = (Circle*)circleListHead->data;

                intArea = ceil((3.14159 * (tmpCircle->r * tmpCircle->r)));

                if (intArea == ceil(area)) {
                    count++;
                }
                circleListHead = circleListHead->next;
            }
            count += findStuffInGroups(tmpGroup->groups, "circle", area);
        }

        groupHead = groupHead->next;
    }

    return count;
}






int findStuffInGroupsForPaths(List* groupList, const char* data) {
    Node* groupHead = groupList->head;

    if (groupHead == NULL) {
        return 0;
    }

    Group* tmpGroup;
    int count = 0;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        Node* pathListHead;
        Path* tmpPath;

        pathListHead = tmpGroup->paths->head;
        // calculate the number of rectangle with the specified area in the non-nested rectangles
        while (pathListHead != NULL) {
            tmpPath = (Path*)pathListHead->data;

            if (strcmp(tmpPath->data, data) == 0) {
                count++;
            }

            pathListHead = pathListHead->next;
        }

        count += findStuffInGroupsForPaths(tmpGroup->groups, data);

        groupHead = groupHead->next;
    }

    return count;
}






int countAttributesInGroups(List* groupList) {
    Node* groupHead = groupList->head;

    if (groupHead == NULL) {
        return 0;
    }

    Group* tmpGroup;
    int count = 0;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;
        // rectangles
        Node* rectHead = tmpGroup->rectangles->head;
        while (rectHead != NULL) {
            Rectangle* rectData = (Rectangle*)rectHead->data;
            count += rectData->otherAttributes->length;
            rectHead = rectHead->next;
        }
        // circles
        Node* circleHead = tmpGroup->circles->head;
        while (circleHead != NULL) {
            Circle* circleData = (Circle*)circleHead->data;
            count += circleData->otherAttributes->length;
            circleHead = circleHead->next;
        }
        // paths
        Node* pathHead = tmpGroup->paths->head;
        while (pathHead != NULL) {
            Path* pathData = (Path*)pathHead->data;
            count += pathData->otherAttributes->length;
            pathHead = pathHead->next;
        }
        

        count += tmpGroup->otherAttributes->length;

        count += countAttributesInGroups(tmpGroup->groups);

        groupHead = groupHead->next;
    }

    return count;
}






void getPathsInGroups(List* paths, List* groupList) {
    Node* groupHead = groupList->head;
    Node* savedHead = groupList->head;
    Node* pathHead;
    Node* savedPathHead;
    Path* pathData;

    if (groupHead == NULL) {
        return;
    }

    Group* tmpGroup;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        pathHead = tmpGroup->paths->head;
        savedPathHead = tmpGroup->paths->head;
        while (pathHead != NULL) {
            pathData = (Path*)pathHead->data;

            insertBack(paths, (void*)pathData);

            pathHead = pathHead->next;
        }
        pathHead = savedPathHead;

        getPathsInGroups(paths, tmpGroup->groups);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}






void getRectanglesInGroups(List* rectangles, List* groupList) {
    Node* groupHead = groupList->head;
    Node* savedHead = groupList->head;
    Node* rectangleHead;
    Node* savedRectangleHead;
    Rectangle* rectangleData;

    if (groupHead == NULL) {
        return;
    }

    Group* tmpGroup;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        savedRectangleHead = tmpGroup->rectangles->head;
        rectangleHead = savedRectangleHead;
        while (rectangleHead != NULL) {
            rectangleData = (Rectangle*)rectangleHead->data;

            insertBack(rectangles, (void*)rectangleData);

            rectangleHead = rectangleHead->next;
        }
        rectangleHead = savedRectangleHead;

        getRectanglesInGroups(rectangles, tmpGroup->groups);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}






void getCirclesInGroups(List* circles, List* groupList) {
    Node* groupHead = groupList->head;
    Node* savedHead = groupList->head;
    Node* circleHead;
    Node* savedCircleHead;
    Circle* circleData;

    if (groupHead == NULL) {
        return;
    }

    Group* tmpGroup;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        savedCircleHead = tmpGroup->circles->head;
        circleHead = savedCircleHead;
        while (circleHead != NULL) {
            circleData = (Circle*)circleHead->data;

            insertBack(circles, (void*)circleData);

            circleHead = circleHead->next;
        }
        circleHead = savedCircleHead;

        getCirclesInGroups(circles, tmpGroup->groups);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}






void getGroupsInGroups(List* totalGroups, Group* groupStruct) {
    if (groupStruct == NULL) {
        return;
    }

    List* groups = groupStruct->groups;
    Node* groupHead = groups->head;
    Node* savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;
        insertBack(totalGroups, (void*)groupData);
        getGroupsInGroups(totalGroups, groupData);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}





void getGroupDetails (xmlNode * a_node, Group *groupStruct) {
    xmlNode *cur_node = NULL;
    if (groupStruct == NULL) {
        return;
    }

    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            // printf("\tnode type: Element, name: %s\n", cur_node->name);
            // if the current element is a group
            if (strcmp((const char *)cur_node->name, "g") == 0) {
                Group* tmpGroup = (Group*)malloc(sizeof(Group));

                List* rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
                tmpGroup->rectangles = rectangles;

                List* circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
                tmpGroup->circles = circles;

                List* paths = initializeList(&pathToString, &deletePath, &comparePaths);
                tmpGroup->paths = paths;

                List* groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
                tmpGroup->groups = groups;

                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpGroup->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\t\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                    if (attrName != NULL) {
                        tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                        strcpy(tempAttribute->name, attrName);
                    } else {
                        tempAttribute->name = "Name was NULL";
                    }
                    strcpy(tempAttribute->value, cont);
                    insertBack(tmpGroup->otherAttributes, (void*)tempAttribute);
                }

                getGroupDetails(cur_node->children, tmpGroup);

                insertBack(groupStruct->groups, (void*)tmpGroup);
            }
            // if the current element is a rectangle
            if (strcmp((const char *)cur_node->name, "rect") == 0) {
                Rectangle* tmpRectangle = (Rectangle*)malloc(sizeof(Rectangle));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpRectangle->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\t\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "x") == 0) {
                        int character;
                        char* number;
                        char* units;
                        int numCount = 0;
                        int unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++) {
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                numCount++;
                            } else {
                                unitsCount++;
                            }
                        }

                        number = (char*)malloc(numCount+1);
                        units = (char*)malloc(unitsCount+1);
                        numCount = unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++){
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                number[numCount] = cont[i];
                                numCount++;
                            } else {
                                units[unitsCount] = cont[i];
                                unitsCount++;
                            }
                        }
                        number[numCount] = '\0';
                        units[unitsCount] = '\0';

                        tmpRectangle->x = atof(number);
                        strcpy(tmpRectangle->units, units);

                        free(number);
                        free(units);
                    }
                    if (strcmp(attrName, "y") == 0) {
                        tmpRectangle->y = atof(cont);
                    }
                    if (strcmp(attrName, "width") == 0) {
                        tmpRectangle->width = atof(cont);
                    }
                    if (strcmp(attrName, "height") == 0) {
                        tmpRectangle->height = atof(cont);
                    }
                    if (strcmp(attrName, "x") != 0 && strcmp(attrName, "y") != 0 && strcmp(attrName, "width") != 0 && strcmp(attrName, "height") != 0) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);
                        insertBack(tmpRectangle->otherAttributes, (void*)tempAttribute);
                    }
                }

                insertBack(groupStruct->rectangles, (void*)tmpRectangle);
            }
            // if the current element is a circle
            if (strcmp((const char *)cur_node->name, "circle") == 0) {
                Circle* tmpCircle = (Circle*)malloc(sizeof(Circle));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpCircle->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\t\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "cx") == 0) {
                        int character;
                        char* number;
                        char* units;
                        int numCount = 0;
                        int unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++) {
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                numCount++;
                            } else {
                                unitsCount++;
                            }
                        }

                        number = (char*)malloc(numCount+1);
                        units = (char*)malloc(unitsCount+1);
                        numCount = unitsCount = 0;

                        for (int i = 0; i < strlen(cont); i++){
                            character = cont[i];
                            if ((character >= 48 && character <= 57) || character == 46) {
                                number[numCount] = cont[i];
                                numCount++;
                            } else {
                                units[unitsCount] = cont[i];
                                unitsCount++;
                            }
                        }
                        number[numCount] = '\0';
                        units[unitsCount] = '\0';

                        tmpCircle->cx = atof(number);
                        strcpy(tmpCircle->units, units);

                        free(number);
                        free(units);
                    }
                    if (strcmp(attrName, "cy") == 0) {
                        tmpCircle->cy = atof(cont);
                    }
                    if (strcmp(attrName, "r") == 0) {
                        tmpCircle->r = atof(cont);
                    }
                    if (strcmp(attrName, "cx") != 0 && strcmp(attrName, "cy") != 0 && strcmp(attrName, "r")) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);
                        insertBack(tmpCircle->otherAttributes, (void*)tempAttribute);
                    }
                }

                insertBack(groupStruct->circles, (void*)tmpCircle);
            }
            // if the current element is a path
            if (strcmp((const char *)cur_node->name, "path") == 0) {
                Path* tmpPath = (Path*)malloc(sizeof(Path));
                List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
                tmpPath->otherAttributes = otherAttributes;
                
                xmlAttr *attr;
                for (attr = cur_node->properties; attr != NULL; attr = attr->next){
                    xmlNode *value = attr->children;
                    char *attrName = (char *)attr->name;
                    char *cont = (char *)(value->content);
                    // printf("\t\tattribute name: %s, attribute value = %s\n", attrName, cont);

                    if (strcmp(attrName, "d") == 0) {
                        if (cont != NULL) {
                            tmpPath = (Path*)realloc(tmpPath, (sizeof(Path) + strlen(cont) + 1));
                            strcpy(tmpPath->data, cont);
                        } else {
                            strcpy(tmpPath->data, "Data was NULL");
                        }
                    }
                    if (strcmp(attrName, "d") != 0) {
                        Attribute* tempAttribute = (Attribute*)malloc(sizeof(Attribute) + strlen(cont) + 1);
                        if (attrName != NULL) {
                            tempAttribute->name = (char*)malloc(strlen(attrName)+1);
                            strcpy(tempAttribute->name, attrName);
                        } else {
                            tempAttribute->name = "Name was NULL";
                        }
                        strcpy(tempAttribute->value, cont);
                        insertBack(tmpPath->otherAttributes, (void*)tempAttribute);
                    }
                }

                insertBack(groupStruct->paths, (void*)tmpPath);
            }
        }
    }
}




// Helper function to writing an SVG struct into a file in SVG format.
xmlDocPtr svgToTree (const SVG* img) {
    xmlNodePtr root_node = NULL;
    root_node = xmlNewNode(NULL, BAD_CAST "svg");
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;
    char temp[50000];

    doc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(doc, root_node);

    if (strlen(img->title) != 0) {
        xmlNewChild(root_node, NULL, BAD_CAST "title", BAD_CAST img->title);
    }
    if (strlen(img->description) != 0) {
        xmlNewChild(root_node, NULL, BAD_CAST "desc", BAD_CAST img->description);
    }

    xmlNsPtr nsptr = xmlNewNs(root_node, BAD_CAST img->namespace, NULL);
    xmlSetNs(root_node, nsptr);

    if (img->otherAttributes == NULL) {
        return doc;
    }
    List* otherAttributes = img->otherAttributes;
    Node* attributeHead = otherAttributes->head;
    Node* savedAttributeHead = otherAttributes->head;
    while (attributeHead != NULL) {
        Attribute* attributedata = (Attribute*)attributeHead->data;
        if (attributedata->name == NULL) {
            return doc;
        }

        xmlNewProp(root_node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

        attributeHead = attributeHead->next;
    }
    attributeHead = savedAttributeHead;

    if (img->rectangles == NULL) {
        return doc;
    }
    // go through the rectangles
    List* rectangles = img->rectangles;
    Node* rectangleHead = rectangles->head;
    Node* savedHead = rectangles->head;
    while (rectangleHead != NULL) {
        Rectangle* rectangleData = (Rectangle*)rectangleHead->data;

        node = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);
        sprintf(temp, "%f", rectangleData->x);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "x", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->y);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "y", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->width);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "width", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->height);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "height", BAD_CAST temp);

        if (rectangleData->otherAttributes == NULL) {
            return doc;
        }
        // add the otherAttributes of the rectangles
        List* otherAttributes = rectangleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return doc;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        rectangleHead = rectangleHead->next;
    }
    rectangleHead = savedHead;

    if (img->circles == NULL) {
        return doc;
    }
    // go through the circles
    List* circles = img->circles;
    Node* circleHead = circles->head;
    savedHead = circles->head;
    while (circleHead != NULL) {
        Circle* circleData = (Circle*)circleHead->data;

        node = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);
        sprintf(temp, "%f", circleData->cx);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "cx", BAD_CAST temp);
        sprintf(temp, "%f", circleData->cy);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "cy", BAD_CAST temp);
        sprintf(temp, "%f", circleData->r);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "r", BAD_CAST temp);

        if (circleData->otherAttributes == NULL) {
            return doc;
        }
        // add the otherAttributes of the circles
        List* otherAttributes = circleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return doc;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        circleHead = circleHead->next;
    }
    circleHead = savedHead;

    if (img->paths == NULL) {
        return doc;
    }
    // go through the paths
    List* paths = img->paths;
    Node* pathHead = paths->head;
    savedHead = paths->head;
    while (pathHead != NULL) {
        Path* pathData = (Path*)pathHead->data;
        if (pathData->data == NULL) {
            return doc;
        }

        node = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
        xmlNewProp(node, BAD_CAST "d", BAD_CAST pathData->data);

        if (pathData->otherAttributes == NULL) {
            return doc;
        }
        // add the otherAttributes of the paths
        List* otherAttributes = pathData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return doc;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        pathHead = pathHead->next;
    }
    pathHead = savedHead;

    if (img->groups == NULL) {
        return doc;
    }
    // go through the groups
    List* groups = img->groups;
    Node* groupHead = groups->head;
    savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;
        node = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);

        if (groupData->otherAttributes == NULL) {
            return doc;
        }
        // add the otherAttributes of the groups
        List* otherAttributes = groupData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return doc;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        groupToTree(groupData, node);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;

    return doc;
}





// Helper function to writing an SVG struct into a file in SVG format.
void groupToTree (Group* groupStruct, xmlNodePtr root_node) {
    if (groupStruct == NULL) {
        return;
    }
    xmlNodePtr node = NULL;
    char temp[50000];

    // go through the rectangles
    if (groupStruct->rectangles == NULL) {
        return;
    }
    List* rectangles = groupStruct->rectangles;
    Node* rectangleHead = rectangles->head;
    Node* savedHead = rectangles->head;
    while (rectangleHead != NULL) {
        Rectangle* rectangleData = (Rectangle*)rectangleHead->data;

        node = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);
        sprintf(temp, "%f", rectangleData->x);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "x", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->y);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "y", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->width);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "width", BAD_CAST temp);
        sprintf(temp, "%f", rectangleData->height);
        strcat(temp, rectangleData->units);
        xmlNewProp(node, BAD_CAST "height", BAD_CAST temp);

        if (rectangleData->otherAttributes == NULL) {
            return;
        }
        // add the otherAttributes of the rectangles
        List* otherAttributes = rectangleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        rectangleHead = rectangleHead->next;
    }
    rectangleHead = savedHead;

    if (groupStruct->circles == NULL) {
        return;
    }
    // go through the circles
    List* circles = groupStruct->circles;
    Node* circleHead = circles->head;
    savedHead = circles->head;
    while (circleHead != NULL) {
        Circle* circleData = (Circle*)circleHead->data;

        node = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);
        sprintf(temp, "%f", circleData->cx);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "cx", BAD_CAST temp);
        sprintf(temp, "%f", circleData->cy);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "cy", BAD_CAST temp);
        sprintf(temp, "%f", circleData->r);
        strcat(temp, circleData->units);
        xmlNewProp(node, BAD_CAST "r", BAD_CAST temp);

        if (circleData->otherAttributes == NULL) {
            return;
        }
        // add the otherAttributes of the circles
        List* otherAttributes = circleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        circleHead = circleHead->next;
    }
    circleHead = savedHead;

    if (groupStruct->paths == NULL) {
        return;
    }
    // go through the paths
    List* paths = groupStruct->paths;
    Node* pathHead = paths->head;
    savedHead = paths->head;
    while (pathHead != NULL) {
        Path* pathData = (Path*)pathHead->data;
        if (pathData->data == NULL) {
            return;
        }

        node = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
        xmlNewProp(node, BAD_CAST "d", BAD_CAST pathData->data);

        if (pathData->otherAttributes == NULL) {
            return;
        }
        // add the otherAttributes of the paths
        List* otherAttributes = pathData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;
        pathHead = pathHead->next;
    }
    pathHead = savedHead;

    if (groupStruct->groups == NULL) {
        return;
    }
    // go through the groups
    List* groups = groupStruct->groups;
    Node* groupHead = groups->head;
    savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;
        node = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);

        if (groupData->otherAttributes == NULL) {
            return;
        }
        // add the otherAttributes of the groups
        List* otherAttributes = groupData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return;
            }

            xmlNewProp(node, BAD_CAST attributedata->name, BAD_CAST attributedata->value);

            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        groupToTree(groupData, node);

        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}





bool validateGroup (Group* groupStruct) {
    if (groupStruct == NULL) {
        return false;
    }

    if (groupStruct->rectangles == NULL || groupStruct->circles == NULL || groupStruct->paths == NULL || groupStruct->groups == NULL || groupStruct->otherAttributes == NULL) {
        return false;
    }

    List* otherAttributes = groupStruct->otherAttributes;
    Node* attributeHead = otherAttributes->head;
    Node* savedAttributeHead = otherAttributes->head;
    while (attributeHead != NULL) {
        Attribute* attributedata = (Attribute*)attributeHead->data;
        if (attributedata->name == NULL) {
            return false;
        }
        attributeHead = attributeHead->next;
    }
    attributeHead = savedAttributeHead;

    // go through the rectangles
    List* rectangles = groupStruct->rectangles;
    Node* rectangleHead = rectangles->head;
    Node* savedHead = rectangles->head;
    while (rectangleHead != NULL) {
        Rectangle* rectangleData = (Rectangle*)rectangleHead->data;

        if (rectangleData->width < 0 || rectangleData->height < 0) {
            return false;
        }
        if (rectangleData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = rectangleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        rectangleHead = rectangleHead->next;
    }
    rectangleHead = savedHead;


    // go through the circles
    List* circles = groupStruct->circles;
    Node* circleHead = circles->head;
    savedHead = circles->head;
    while (circleHead != NULL) {
        Circle* circleData = (Circle*)circleHead->data;

        if (circleData->r < 0) {
            return false;
        }
        if (circleData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = circleData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        circleHead = circleHead->next;
    }
    circleHead = savedHead;


    // go through the paths
    List* paths = groupStruct->paths;
    Node* pathHead = paths->head;
    savedHead = paths->head;
    while (pathHead != NULL) {
        Path* pathData = (Path*)pathHead->data;

        if (pathData->data == NULL) {
            return false;
        }
        if (pathData->otherAttributes == NULL) {
            return false;
        }

        List* otherAttributes = pathData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        pathHead = pathHead->next;
    }
    pathHead = savedHead;


    // go through the groups
    List* groups = groupStruct->groups;
    Node* groupHead = groups->head;
    savedHead = groups->head;
    while (groupHead != NULL) {
        Group* groupData = (Group*)groupHead->data;

        if (groupData->rectangles == NULL || groupData->circles == NULL || groupData->paths == NULL || groupData->groups == NULL || groupData->otherAttributes == NULL) {
            return false;
        }

        // add the otherAttributes of the groups
        List* otherAttributes = groupData->otherAttributes;
        Node* attributeHead = otherAttributes->head;
        Node* savedAttributeHead = otherAttributes->head;
        while (attributeHead != NULL) {
            Attribute* attributedata = (Attribute*)attributeHead->data;
            if (attributedata->name == NULL) {
                return false;
            }
            attributeHead = attributeHead->next;
        }
        attributeHead = savedAttributeHead;

        if (!validateGroup(groupData)) {
            return false;
        }

        groupHead = groupHead->next;
    }
    groupHead = savedHead;

    return true;
}





void getLengthsInGroups (List* groupList, int *numRect, int *numCirc, int *numPaths, int *numGroups) {
    Node* groupHead = groupList->head;
    Node* savedHead = groupList->head;
    Group* tmpGroup;

    while (groupHead != NULL) {
        tmpGroup = (Group*)groupHead->data;

        
        *numRect += tmpGroup->rectangles->length;
        *numCirc += tmpGroup->circles->length;
        *numPaths += tmpGroup->paths->length;
        *numGroups += tmpGroup->groups->length;

        getLengthsInGroups(tmpGroup->groups, numRect, numCirc, numPaths, numGroups);
        groupHead = groupHead->next;
    }
    groupHead = savedHead;
}





// takes a filename and returns object of numRects, numCircles, numPaths, numGroups, etc
char* fileSVGtoJSON(const char* fileName, const char* schemaFile) {
    char* temp;
    // SVG *svgStruct = createValidSVG(fileName, schemaFile);
    SVG *svgStruct = createValidSVG(fileName, "svg.xsd");
    temp = SVGtoJSON(svgStruct);
    deleteSVG(svgStruct);

    return temp;
}





char* svgViewPannel(const char* fileName) {
    char* JSONString;
    char* temp;
    char* temp2;
    SVG *svgStruct = createValidSVG(fileName, "svg.xsd");

    if (svgStruct != NULL) {
        JSONString = (char*)malloc(sizeof(svgStruct->title) + sizeof(svgStruct->description) + 100);
        sprintf(JSONString, "[[{\"title\":\"%s\",\"description\":\"%s\"}],", svgStruct->title, svgStruct->description);

        temp2 = rectListToJSON(svgStruct->rectangles);
        temp = (char*)malloc(strlen(temp2) + 2);
        sprintf(temp, "%s,", temp2);
        JSONString = (char*)realloc(JSONString, (strlen(JSONString) + strlen(temp) + 1));
        strcat(JSONString, temp);
        free(temp);
        free(temp2);

        temp2 = circListToJSON(svgStruct->circles);
        temp = (char*)malloc(strlen(temp2) + 2);
        sprintf(temp, "%s,", temp2);
        JSONString = (char*)realloc(JSONString, (strlen(JSONString) + strlen(temp) + 1));
        strcat(JSONString, temp);
        free(temp);
        free(temp2);

        temp2 = pathListToJSON(svgStruct->paths);
        temp = (char*)malloc(strlen(temp2) + 2);
        sprintf(temp, "%s,", temp2);
        JSONString = (char*)realloc(JSONString, (strlen(JSONString) + strlen(temp) + 1));
        strcat(JSONString, temp);
        free(temp);
        free(temp2);

        temp2 = groupListToJSON(svgStruct->groups);
        temp = (char*)malloc(strlen(temp2) + 2);
        sprintf(temp, "%s]", temp2);
        JSONString = (char*)realloc(JSONString, (strlen(JSONString) + strlen(temp) + 1));
        strcat(JSONString, temp);
        free(temp);
        free(temp2);

        deleteSVG(svgStruct);
        xmlCleanupParser();
        printf("%s\n", JSONString);
    } else {
        JSONString = (char*)malloc(100);
        sprintf(JSONString, "[]");
    }

    return JSONString;
}





char* getOtherAttributes(const char* fileName, const char* componentName, int componentNum) {
    char* temp = NULL;
    SVG *svgStruct = createValidSVG(fileName, "svg.xsd");
    int i = 0;

    if (strcmp(componentName, "Rectangle") == 0) {
        List* rectangles = svgStruct->rectangles;
        Node* rectangleHead = rectangles->head;
        Node* savedHead = rectangles->head;
        while (rectangleHead != NULL) {
            Rectangle* rectangleData = (Rectangle*)rectangleHead->data;
            if (i == componentNum) {
                temp = attrListToJSON(rectangleData->otherAttributes);
            }
            rectangleHead = rectangleHead->next;
            i++;
        }
        rectangleHead = savedHead;
    } else if (strcmp(componentName, "Circle") == 0) {
        List* circles = svgStruct->circles;
        Node* head = circles->head;
        Node* savedHead = circles->head;
        while (head != NULL) {
            Circle* data = (Circle*)head->data;
            if (i == componentNum) {
                temp = attrListToJSON(data->otherAttributes);
            }
            head = head->next;
            i++;
        }
        head = savedHead;
    } else if (strcmp(componentName, "Path") == 0) {
        List* paths = svgStruct->paths;
        Node* head = paths->head;
        Node* savedHead = paths->head;
        while (head != NULL) {
            Path* data = (Path*)head->data;
            if (i == componentNum) {
                temp = attrListToJSON(data->otherAttributes);
            }
            head = head->next;
            i++;
        }
        head = savedHead;
    } else if (strcmp(componentName, "Group") == 0) {
        List* groups = svgStruct->groups;
        Node* head = groups->head;
        Node* savedHead = groups->head;
        while (head != NULL) {
            Group* data = (Group*)head->data;
            if (i == componentNum) {
                temp = attrListToJSON(data->otherAttributes);
            }
            head = head->next;
            i++;
        }
        head = savedHead;
    }

    deleteSVG(svgStruct);
    return temp;
}





int createSVGfromJSON (const char* svgString, const char* filename) {
    int valid = 0;
    SVG *svgStruct = JSONtoSVG(svgString);
    if (validateSVG(svgStruct, "svg.xsd")) {
        printf("SVG struct passed validation\n");
        writeSVG(svgStruct, filename);
        valid = 1;
    } else {
        printf("SVG struct did not pass validation\n");
        valid = 0;
    }

    return valid;
}





int JSONAddComponent(const char* shapeString, const char* filename, int type) {
    SVG *svgStruct = createSVG(filename);
    int valid = 0;

    if (type == 1) {
        Circle *circle = JSONtoCircle(shapeString);
        insertBack(svgStruct->circles, (void*)circle);
    } else if (type == 2) {
        Rectangle *rect = JSONtoRect(shapeString);
        insertBack(svgStruct->rectangles, (void*)rect);
    }

    if (validateSVG(svgStruct, "svg.xsd")) {
        printf("SVG struct passed validation\n");
        writeSVG(svgStruct, filename);
        valid = 1;
    } else {
        printf("SVG struct did not pass validation\n");
        valid = 0;
    }

    return valid;    
}





int scaleShapes(const char* filename, const char* shapeType, float scaleFactor) {
    SVG *svgStruct = createSVG(filename);

    if (strcmp(shapeType, "Rectangle") == 0) {
        List* rectangles = getRects(svgStruct);
        Node* rectangleHead = rectangles->head;
        Node* savedHead = rectangles->head;
        while (rectangleHead != NULL) {
            Rectangle* rectangleData = (Rectangle*)rectangleHead->data;
            rectangleData->width = rectangleData->width * scaleFactor;
            rectangleData->height = rectangleData->height * scaleFactor;
            rectangleHead = rectangleHead->next;
        }
        rectangleHead = savedHead;   
    } else if (strcmp(shapeType, "Circle") == 0) {
        List* circles = getCircles(svgStruct);
        Node* circleHead = circles->head;
        Node* savedHead = circles->head;
        while (circleHead != NULL) {
            Circle* circleData = (Circle*)circleHead->data;
            circleData->r = circleData->r * scaleFactor;
            circleHead = circleHead->next;
        }
        circleHead = savedHead;
    }

    int valid = 0;
    if (validateSVG(svgStruct, "svg.xsd")) {
        printf("SVG struct passed validation\n");
        writeSVG(svgStruct, filename);
        valid = 1;
    } else {
        printf("SVG struct did not pass validation\n");
        valid = 0;
    }

    return valid;
}











// int main(int argc, char **argv)
// {
//     if (argc != 2) {
//         return(1);
//     }
//     // char* svgStr;
//     // char* circleStr;

//     // **************************** Create and print SVG: ***********************
//     printf("*********************** Create SVG: ***********************\n\n");
//     // SVG *svgStruct = createSVG(argv[1]);
//     SVG *svgStruct = createValidSVG(argv[1], "svg.xsd");

//     // svgStr = SVGToString(svgStruct); // this will print all the lists in the SVG, and then do the group lists
//     // printf("\n*********************** Print SVG: ***********************\n\n");
//     // printf("%s", svgStr);
//     // free(svgStr);


//     // **************************** Structs with specific info: ***********************
//     printf("\n*********************** Structs with specific info: ***********************\n");
//     int printval;
//     printval = numRectsWithArea(svgStruct, 716404.00);
//     printf("numRectsWithArea of 716404.00 = %d\n", printval);

//     printval = numCirclesWithArea(svgStruct, 20.00);
//     printf("numCirclesWithArea of 20.00 = %d\n", printval);

//     printval = numPathsWithdata(svgStruct, "M200,300 Q400,50 600,300 T1000,300");
//     printf("numPathsWithdata of 'M200,300 Q400,50 600,300 T1000,300' = %d\n", printval);

//     printval = numGroupsWithLen(svgStruct, 2);
//     printf("numGroupsWithLen of 20 = %d\n", printval);
    
//     printval = numAttr(svgStruct);
//     printf("numAttr in struct = %d\n", printval);

//     // **************************** Saving the SVG to a file: ***********************
//     printf("\n*********************** Saving the SVG to a file: ***********************\n");
//     writeSVG(svgStruct, "savedSVG.svg");

//     // **************************** Validating SVG: ***********************
//     printf("\n*********************** Validating SVG: ***********************\n");
//     // List* circles = svgStruct->circles;
//     // Node* circleHead = circles->head;
//     // Circle* circleData = (Circle*)circleHead->data;
//     // circleData->otherAttributes = NULL;
//     // svgStruct->circles = NULL;
//     if (validateSVG(svgStruct, "svg.xsd")) {
//         printf("SVG struct passed validation\n");
//     } else {
//         printf("SVG struct did not pass validation\n");
//     }

//     // **************************** setAttribute: ***********************
//     printf("\n*********************** setAttribute: ***********************\n");
//     Attribute* newAttr;
//     newAttr = malloc(sizeof(Attribute) + strlen("1.5")+1);

//     newAttr->name = (char*)malloc(strlen("test")+1);
//     strcpy(newAttr->name, "test");
//     strcpy(newAttr->value, "1.5");

//     if (!setAttribute(svgStruct, RECT, 0, newAttr)) {
//         printf("setAttribute -> invalid attr\n");
//         free(newAttr->name);
//         free(newAttr);
//     }

//     // **************************** Add Component: ***********************
//     printf("\n*********************** Add Component: ***********************\n");
//     Path* tmpPath = (Path*)malloc(sizeof(Path));
//     List* otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
//     tmpPath->otherAttributes = otherAttributes;
//     tmpPath = (Path*)realloc(tmpPath, (sizeof(Path) + strlen("Test data") + 1));
//     strcpy(tmpPath->data, "Test data");

//     addComponent(svgStruct, PATH, (void*)tmpPath);


//     // **************************** Saving the SVG to a file: ***********************
//     printf("\n*********************** Saving the SVG to a file: ***********************\n");
//     writeSVG(svgStruct, "savedSVG.svg");

//     // **************************** toJSON Functions: ***********************
//     printf("\n*********************** toJSON Functions: ***********************\n");
//     char* temp;

//     Attribute* tempStruct = (Attribute*)malloc(sizeof(Attribute) + strlen("content") + 1);
//     tempStruct->name = "fill";
//     strcpy(tempStruct->value, "red");
//     temp = attrToJSON(tempStruct);
//     printf("%s\n", temp);
//     free(temp);
//     free(tempStruct);

//     List* circles = svgStruct->circles;
//     Node* circleHead = circles->head;
//     if (circleHead != NULL) {
//         Circle* circleData = (Circle*)circleHead->data;
//         temp = circleToJSON(circleData);
//     } else {
//         temp = circleToJSON(NULL);
//     }
//     printf("%s\n", temp);
//     free(temp);

//     List* rectangles = svgStruct->rectangles;
//     Node* rectangleHead = rectangles->head;
//     if (rectangleHead != NULL) {
//         Rectangle* rectangleData = (Rectangle*)rectangleHead->data;
//         temp = rectToJSON(rectangleData);
//     } else {
//         temp = rectToJSON(NULL);
//     }
//     printf("%s\n", temp);
//     free(temp);

//     List* paths = svgStruct->paths;
//     Node* pathHead = paths->head;
//     if (pathHead != NULL) {
//         Path* pathData = (Path*)pathHead->data;
//         temp = pathToJSON(pathData);
//     } else {
//         temp = pathToJSON(NULL);
//     }
//     printf("%s\n", temp);
//     free(temp);

//     List* groups = svgStruct->groups;
//     Node* groupHead = groups->head;
//     if (groupHead != NULL) {
//         Group* groupData = (Group*)groupHead->data;
//         temp = groupToJSON(groupData);
//     } else {
//         temp = groupToJSON(NULL);
//     }
//     printf("%s\n", temp);
//     free(temp);

//     Rectangle* rectangleData = (Rectangle*)rectangleHead->data;
//     temp = attrListToJSON(rectangleData->otherAttributes);
//     printf("%s\n", temp);
//     free(temp);

//     temp = circListToJSON(svgStruct->circles);
//     printf("%s\n", temp);
//     free(temp);

//     temp = rectListToJSON(svgStruct->rectangles);
//     printf("%s\n", temp);
//     free(temp);

//     temp = pathListToJSON(svgStruct->paths);
//     printf("%s\n", temp);
//     free(temp);

//     temp = groupListToJSON(svgStruct->groups);
//     printf("%s\n", temp);
//     free(temp);

//     temp = SVGtoJSON(svgStruct);
//     printf("%s\n", temp);
//     free(temp);


//     // **************************** Accessor functions: ***********************
//     // printf("*********************** Accessor functions: ***********************\n\n");
//     // List* paths = getPaths(svgStruct);
//     // Node* pathHead = paths->head;
//     // Node* savedHead = paths->head;
//     // while (pathHead != NULL) {
//     //     Path* pathData = (Path*)pathHead->data;
//     //     strcpy(pathData->data, "newData");
//     //     pathHead = pathHead->next;
//     // }
//     // pathHead = savedHead;

//     // List* rectangles = getRects(svgStruct);
//     // Node* rectangleHead = rectangles->head;
//     // Node* savedHead = rectangles->head;
//     // while (rectangleHead != NULL) {
//     //     Rectangle* rectangleData = (Rectangle*)rectangleHead->data;
//     //     rectangleData->x = 420;
//     //     rectangleData->y = 69;
//     //     rectangleHead = rectangleHead->next;
//     // }
//     // rectangleHead = savedHead;

//     // List* circles = getCircles(svgStruct);
//     // Node* circleHead = circles->head;
//     // Node* savedHead = circles->head;
//     // while (circleHead != NULL) {
//     //     Circle* circleData = (Circle*)circleHead->data;
//     //     circleData->cx = 420;
//     //     circleData->cy = 69;
//     //     circleHead = circleHead->next;
//     // }
//     // circleHead = savedHead;

//     // List* groups = getGroups(svgStruct);
//     // Node* groupHead = groups->head;
//     // Node* savedHead = groups->head;
//     // while (groupHead != NULL) {
//     //     Group* groupData = (Group*)groupHead->data;
//     //     groupData->circles->length = 5;
//     //     groupHead = groupHead->next;
//     // }
//     // groupHead = savedHead;

//     // groupHead = svgStruct->groups->head;
//     // savedHead = groupHead;
//     // while (groupHead != NULL) {
//     //     Group* groupData = (Group*)groupHead->data;
//     //     printf("length = %d\n", groupData->circles->length);
//     //     groupHead = groupHead->next;
//     // }
//     // groupHead = savedHead;
//     // printf("*******************************************************************\n");

    
//     // **************************** Delete SVG: ***********************
//     printf("\n*********************** Delete SVG: ***********************\n\n");
//     deleteSVG(svgStruct);
//     xmlCleanupParser();

//     return 0;
// }