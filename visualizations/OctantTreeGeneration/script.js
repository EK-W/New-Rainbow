window.addEventListener("load", function() {

const SVGNS = "http://www.w3.org/2000/svg";
const COLOR_CUBE_INSET = 0.125;

let dimensionForm = document.getElementById("dimension-form");
let field = document.getElementById("color-field");
let fieldControls = document.getElementById("field-controls");

let fieldDataHistory = [];
let dataHistoryIndex;
let fieldData = {};


if(sessionStorage.getItem("x-dimension") !== null) {
	setupField(sessionStorage.getItem("x-dimension"), sessionStorage.getItem("y-dimension"));
}

dimensionForm.elements["apply-button"].addEventListener("click", function() {
	let xDimensionInput = dimensionForm.elements["x-dimension-input"];
	let yDimensionInput = dimensionForm.elements["y-dimension-input"];

	if(setupField(xDimensionInput.value, yDimensionInput.value)) {
		dimensionForm.classList.remove("bad-apply");	
	} else {
		dimensionForm.classList.add("bad-apply");
	}
});

// fieldControls.elements["field-last-step-button"].addEventListener("click", () => {

// });

fieldControls.elements["field-next-step-button"].addEventListener("click", () => {

});

function getDataItemInLevel(levelDat, x, y) {
	if(x < 0 || x >= levelDat.width || y < 0 || y >= levelDat.height) {
		return null;
	}
	const index = (x * levelDat.height) + y;
	return levelDat.items[index];
}

function setupField(dimX, dimY) {
	// validate dimensions
	if(typeof dimX !== "number") dimX = Number.parseInt(dimX);
	if(typeof dimY !== "number") dimY = Number.parseInt(dimY);

	if(Number.isNaN(dimX) || Number.isNaN(dimY) || dimX <= 0 || dimX > 256 || dimY <= 0 || dimY > 256) {
		return false;
	}

	// update places where dimensions are stored
	sessionStorage.setItem("x-dimension", dimX);
	sessionStorage.setItem("y-dimension", dimY);
	dimensionForm.elements["x-dimension-input"].value = dimX;
	dimensionForm.elements["y-dimension-input"].value = dimY;

	// delete anything that might be in the color field
	while(field.firstChild) {
		field.lastChild.remove();
	}

	let levelsData = [];
	let level = 0;


	levelsData[level] = {
		width: dimX,
		height: dimY,
		items: []
	};
	for(let x = 0; x < dimX; x++) {
		for(let y = 0; y < dimY; y++) {
			let sqr = {
				levelX: x,
				levelY: y,
				globalX: x,
				globalY: y,
				xSpan: 1,
				ySpan: 1
			};

			sqr.element = document.createElementNS(SVGNS, "rect");
			sqr.element.setAttribute("x", x + COLOR_CUBE_INSET);
			sqr.element.setAttribute("y", y + COLOR_CUBE_INSET);
			sqr.element.setAttribute("width", 1 - (2 * COLOR_CUBE_INSET));
			sqr.element.setAttribute("height", 1 - (2 * COLOR_CUBE_INSET));
			sqr.element.setAttribute("fill", "black");

			field.appendChild(sqr.element);
			levelsData[level].items.push(sqr);
		}
	}

	level++;

	let levelDivisor = Math.pow(2, level);
	levelsData[level] = {
		width: Math.ceil(dimX / levelDivisor),
		height: Math.ceil(dimY / levelDivisor),
		items: []
	};

	for(let levelX = 0; levelX < levelsData[level].width; levelX++) {
		for(let levelY = 0; levelY < levelsData[level].height; levelY++) {
			let oct = {
				levelX,
				levelY,
				globalX: levelX * levelDivisor,
				globalY: levelY * levelDivisor,
				children: []
			};

			oct.element = document.createElementNS(SVGNS, "g");
			
			let maxX = oct.globalX;
			let maxY = oct.globalY;

			for(let dx = 0; dx < 2; dx++) {
				for(let dy = 0; dy < 2; dy++) {
					const child = getDataItemInLevel(
						levelsData[level - 1],
						(oct.levelX * 2) + dx,
						(oct.levelY * 2) + dy
					);

					if(child === null) {
						continue;
					}
					
					oct.children.push(child);

					oct.element.appendChild(child.element);

					maxX = Math.max(maxX, child.globalX + child.xSpan);
					maxY = Math.max(maxY, child.globalY + child.ySpan);
				}
			}

			oct.xSpan = maxX - oct.globalX;
			oct.ySpan = maxY - oct.globalY;

			field.appendChild(oct.element);
			levelsData[level].items.push(oct);
		}
	}
}


// function processNextLevel(lastLevelXDim, lastLevelYDim)

/*class ColorOctant {
	constructor(minColor, maxSpan) {
		this.minColor = minColor;
		this.maxColor = { minColor.x + maxSpan.x - 1, minColor.y + maxSpan.y - 1 };
		this.children = [];

		const idealMaxSpan = {
			x: Math.pow(2, Math.ceil(Math.log2(maxSpan.x))),
			y: Math.pow(2, Math.ceil(Math.log2(maxSpan.x))),
		};
		const idealChildrenMaxSpan = {
			x: idealMaxSpan.x / 2, // guaranteed to be divisible by 2 because idealMaxSpans are powers of 2.
			y: idealMaxSpan.y / 2
		};

		for(let x_iter = 0; x_iter <= 1; x_iter++) {
			for(let y_iter = 0; y_iter <= 1; y_iter++) {
				const childDeltaMinX = {
					x: idealChildrenMaxSpan.x * x_iter,
					y: idealChildrenMaxSpan.y * y_iter
				};
				const childMinColor = {
					x: this.minColor.x + childDeltaMinX,
					y: this.minColor.y + 
				};
				const childMaxSpan = {
					x: (childMinColor.x + idealChildrenMaxSpan.x) > maxSpan.
				}
			}
		}
	}
}*/


function deepCopyObject(obj) {
	let newObj = {};
	for(let i in obj) {
		if(typeof obj[i] === "object") {
			newObj[i] = deepCopyObject(obj[i]);
		} else {
			newObj[i] = obj[i];
		}
	}
	return newObj;
}

function saveCurrentFieldState() {
	fieldDataHistory.push(deepCopyObject(fieldData));
	dataHistoryIndex = fieldDataHistory.length - 1;
}

// function loadFieldState(index) {
// 	const toLoad = fieldDataHistory[index];
// 	dataHistoryIndex = index;


// }

function resetColorField(width, height) {
	const colorShrink = 0.125;
	const svgMaxDimension = 500;

	let field = document.getElementById("color-field");
	let fieldControls = document.getElementById("field-controls");

	

	while(fieldControls.firstChild) {
		fieldControls.lastChild.remove();
	}



	field.setAttribute("viewBox", `0, 0, ${width}, ${height}`);
	const svgWidth = (width > height)? svgMaxDimension : (svgMaxDimension * width / height);
	const svgHeight = (width > height)? (svgMaxDimension * height / width) : svgMaxDimension;

	field.setAttribute("width", svgWidth);
	field.setAttribute("height", svgHeight);

	let colorData = [];

	for(let x = 0; x < width; x++) {
		colorData[x] = [];
		for(let y = 0; y < height; y++) {
			
		}
	}


}

});