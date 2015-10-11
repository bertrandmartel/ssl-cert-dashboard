/**
The MIT License (MIT)

Copyright (c) 2015 Bertrand Martel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**
* File        : login.js
* Description : Digest login http call in XHR 
*/

//nonce try count
var nonce_count=1;

//digest realm used
var realm;

//quality of protection value
var qop;

//nonce value
var nonce;

//opaque value
var opaque;

//digest algorithm
var algorithm;

//cookie session value
var hsid;

function init() {

	if (typeof headers != "undefined"){

		console.log("authentication : " + headers);
		console.log("cookie         : " + document.cookie);
		console.log("---------------------------------------------------------")
		realm = getAuthenticationElement(headers,"realm");
		console.log("realm     : " + realm);
		qop = getAuthenticationElement(headers,"qop");
		console.log("qop       : " + qop);
		nonce = getAuthenticationElement(headers,"nonce");
		console.log("nonce     : " + nonce);
		opaque=getAuthenticationElement(headers,"opaque");
		console.log("opaque    : " + opaque);
		console.log("stale     : " + getAuthenticationElement(headers,"stale"));
		algorithm=getAuthenticationElement(headers,"algorithm");
		console.log("algorithm : " + algorithm);
		hsid = getCookie("HSID");
		console.log("cookie    : " + hsid);
		console.log("---------------------------------------------------------")

		document.getElementById('infoMessage').value="";
	}
}

/**
* Create new XML http request object
*/
function getXHR() {

	if (typeof XMLHttpRequest != 'undefined') {
		return new XMLHttpRequest();
	}

	try {
		return new ActiveXObject("Msxml2.XMLHTTP");
	} catch (e) {
		try {
			return new ActiveXObject("Microsoft.XMLHTTP");
		} catch (e) {
		}
	}
	return false;

}

/**
* Random generator for short string
*/
function makeid() {

	var text = "";
	var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	for( var i=0; i < 20; i++ )
		text += possible.charAt(Math.floor(Math.random() * possible.length));

	return text;

}

/**
* Get authentication header element by name
*/
function getAuthenticationElement(headersStr,authname){

	var re = new RegExp("\\s.*");
	var name = authname + "=";
	var ca = headersStr.match(re)[0].substring(1).split(',');
	for(var i=0; i<ca.length; i++) {
		var c = ca[i];
		if (c.indexOf(name) == 0)
		return c.substring(name.length,c.length).replace(new RegExp('\"', 'g'),'');
	}
	return "";

}

/**
* Get cookie value by name
*/
function getCookie(cname) {

	var name = cname + "=";
	var ca = document.cookie.split(';');
	for(var i=0; i<ca.length; i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1);
		if (c.indexOf(name) == 0) return c.substring(name.length,c.length);
	}
	return "";

}

/**
* 8 zeroes padding
*/
function padToEight(number) {

	if (number<=99999999) { 
		number = ("0000000"+number).slice(-8);
	}
	return number;

}
/**
* authentication process on login button click
*/
function login(){

	var username=document.getElementById("user").value;
	var password=document.getElementById("password").value;
	var cnonce =makeid();
	var first_part=md5(username + ":" + window.atob(realm) + ":" + password);
	var last_part=md5("GET:/registration");
	var response_digest=md5(first_part + ":" + nonce + ":" + padToEight(nonce_count)  + ":" + cnonce + ":" + qop + ":" + last_part);

	console.log("first_part      : " + first_part);
	console.log("last_part       : " + last_part);
	console.log("response_digest : " + response_digest);

	var response_header_digest='algorithm="' + algorithm + '",opaque="' + opaque + '",digest-uri="/registration",username="' + username + '",qop="' + qop + '",nc=' + padToEight(nonce_count) + ',response=' + response_digest + ',cnonce=' + cnonce;
	var response_header_cookie="Cookie: HSID=" + hsid + "; Domain=127.0.0.1; Path=/;";

	nonce_count++;

	console.log(response_header_digest);
	console.log(response_header_cookie);

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.status==200) {
			document.location.href="/dashboard";
		}

	}

	xhr.open("GET","/registration",true);
	xhr.setRequestHeader("Authorization", response_header_digest);
	xhr.send();
}