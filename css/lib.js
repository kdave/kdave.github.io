function myclick() {
	alert("CLICK");
}

function setJustifyLeft() {
	document.getElementsByTagName('body')[0].style.marginLeft='2em';
	document.getElementsByTagName('body')[0].style.marginRight='auto';
}

function setJustifyRight() {
	document.getElementsByTagName('body')[0].style.marginRight='2em';
	document.getElementsByTagName('body')[0].style.marginLeft='auto';
}

function setJustifyCenter() {
	document.getElementsByTagName('body')[0].style.marginRight='auto';
	document.getElementsByTagName('body')[0].style.marginLeft='auto';
}

function setFontSize(size) {
	var elems=document.getElementsByTagName('p');
	var sizes=['initial', 'smaller', 'initial', 'larger'];
	for (i=0; i<elems.length; i++) {
		elems[i].style.fontSize = sizes[size];
	}
}
