var userGlobalList = [];
var tableUser;
var selectedUser;
var selectedRowObject;

function init(){

	$(document).ready(function() {
		initUserTable();
		updateUserConf();
	});
}

function updateUserConf(){

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4 && xhr.status==200) {

			var resp = JSON.parse(xhr.responseText);
			userGlobalList=resp["data"];
			updateUserList(userGlobalList);
		}
	}

	xhr.open("GET","/users",true);
	xhr.send();
}

function createNewUser(){
	$('#createuser').dialog('open');
}

function deleteUser(){

	console.log("delete user");

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4 && xhr.status==200) {

			for (var i = userGlobalList.length-1; i >=0;i--){

				if (userGlobalList[i]["username"]==selectedUser){
					userGlobalList.splice(i,1);
				}
			}
			tableUser.fnDeleteRow(selectedRowObject);
		}
	}

	xhr.open("DELETE","/users",true);
	xhr.send('{"username":"' + selectedUser + '"}');
}

function createUser(){

	var username = document.getElementById("new_username").value;
	var password = document.getElementById("password").value;
	var role = 0;

	if ($('input.checkbox_role').is(':checked')) {
		role=1;
	}

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4  && xhr.status==200) {

			var newUser={ "username" : username, "password" : password, "role" : role ,"creationDate": Date.now(),"lastLoginDate": Date.now()};
			userGlobalList.push(newUser);
			reloadTable();

		}
	}

	xhr.open("PUT","/users",true);
	xhr.send('{"username":"' + username + 
			'","password":"' + password + 
			'","role":' + role + '}');

}

function logout(){

	var xhr = getXHR();

	xhr.onreadystatechange=function() {
		if (xhr.status==200) {
			document.location.href="/login";
		}
	}

	xhr.open("GET","/logout",true);
	xhr.send();
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
		} 
		catch (e) {
		}
	}
	return false;
}

function initUserTable(){

		var selected = [];
    tableUser = $('#users').dataTable({
    	"iDisplayLength": 50,
    	"bLengthChange": false,
        "bPaginate": false,
        "bJQueryUI": true,
        "bRetrieve": true,
        "bDestroy" : true,
    	"aaData"   : [],
    	"data": userGlobalList,
		aoColumns : 
		[
			{"sWidth": "20%"},
			{"sWidth": "20%"},
			{"sWidth": "20%"},
			{"sWidth": "20%"},
			{
				"sWidth"        : "10%",
				"orderable"     : false,
				"data"          : null ,
				"defaultContent": '',
				"mRender"       : function(dataIn,type,full){
					return '<input style=\'float:right;\' class=\"deleteUsers\" type=\"button\" value=\"\"/>';
				}
			}
	    ],
	    "order": [[0, 'dsc']]
    });
	}

function formatDate(mydate) {

	var myDateObj = new Date(mydate);

	var dd = myDateObj.getDate();

	if(dd<10){

		dd="0" + dd;	
	}
	var mm1 = myDateObj.getMonth()+1; //January is 0!

	if (mm1<10){

		mm1="0" + mm1;
	}

	var yyyy = myDateObj.getFullYear();
	var hh = myDateObj.getHours();

	if (hh<10){

		hh = "0" +hh;
	}

	var mm2 = myDateObj.getMinutes();

	if (mm2<10){

		mm2="0" + mm2;
	}

	var ss = myDateObj.getSeconds();

	if (ss <10){

		ss ="0" +ss;
	}

	var ret = dd + "/" + mm1 + "/" + yyyy + " " + hh +":" + mm2 + ":" + ss;
	return ret;
}
function reloadTable() {

	tableUser.fnClearTable();
	tableUser.fnDestroy();
	var userTemp = userGlobalList;
	userGlobalList=[];
	initUserTable();
	updateUserList(userTemp);
	userGlobalList=userTemp;
}


function updateUserList(userList){

	var oSettings = tableUser.fnSettings();

	tableUser.fnClearTable(this);

	for (var i = 0;i < userList.length;i++){

		tableUser.oApi._fnAddData(oSettings,
			[
				userList[i]["username"]                  ,
				userList[i]["role"]            ,
				formatDate(userList[i]["creationDate"]) ,
				formatDate(userList[i]["lastLoginDate"])   ,
				""
			]
			);
	}
	
	oSettings.aiDisplay = oSettings.aiDisplayMaster.slice();
	tableUser.fnDraw();
	tableUser.oApi._fnProcessingDisplay(oSettings, false);

	var count = 1;
	var offsetGenerate=9;
	var offsetDownload=10;

	$('#users tbody tr td').on('click','.deleteUsers',function(){

		selectedRowObject=tableUser.fnGetPosition( $(this).closest('tr')[0] );
		$( "#deleteDialog" ).dialog( "open" );
	});

	$('#users tbody tr').on('click', function () {

		var rowIndex = tableUser.fnGetPosition( $(this).closest('tr')[0] );
		var aData = tableUser.fnGetData(rowIndex);
		selectedUser=aData[0];
	});
}