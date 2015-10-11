
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

var sslCerts = [];

var selectedRowObject;

var collapsed = false;
var selectedSerial="";
var selectedCertType ="";
var caFilterIndex=0;
var selectedCertUse ="";

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

/* websocket variable*/
	//var ws;
var tableCert;
var selectedCert;
var selectedIsCa=false;

function init(){

	$(document).ready(function() {
		initSslTable();
		updateSslConf();
	});

	initDate();
}

	// initialize filter date
	function initDate(){

		var todayDateFormat = new Date();
	var dd = todayDateFormat.getDate();
	var mm = todayDateFormat.getMonth()+1; //January is 0!
	var yyyy = todayDateFormat.getFullYear();

	if(dd<10) {
	    dd='0'+dd;
	} 

	if(mm<10) {
	    mm='0'+mm;
	} 

	var tomorrow = new Date();
	tomorrow.setDate(todayDateFormat.getDate()+1);

	today = mm+'/'+dd+'/'+yyyy;

	var dd1 = tomorrow.getDate();
	var mm1 = tomorrow.getMonth()+1; //January is 0!
	var yyyy1 = tomorrow.getFullYear();

	if(dd1<10) {
	    dd1='0'+dd1;
	} 

	if(mm1<10) {
	    mm1='0'+mm1;
	} 

	var today1 = mm1+'/'+dd1+'/'+yyyy1;	

	var tenYear = new Date();
	tenYear.setDate(todayDateFormat.getDate()+3650);
	
	var dd1 = tenYear.getDate();
	var mm1 = tenYear.getMonth()+1; //January is 0!
	var yyyy1 = tenYear.getFullYear();

	if(dd1<10) {
	    dd1='0'+dd1;
	} 

	if(mm1<10) {
	    mm1='0'+mm1;
	} 

	var tenYearFormat = mm1+'/'+dd1+'/'+yyyy1;	

	document.getElementById("caCertDatepicker1").value=today;
	document.getElementById("caCertDatepicker2").value=tenYearFormat;

	document.getElementById("caCert-single-input").value="7:00";
	document.getElementById("caCert-single-input2").value="23:00";

	document.getElementById("CertDatepicker1").value=today;
	document.getElementById("CertDatepicker2").value=tenYearFormat;

	document.getElementById("Cert-single-input").value="7:00";
	document.getElementById("Cert-single-input2").value="23:00";
	}

function updateSslConf(){

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4 && xhr.status==200) {

			var resp = JSON.parse(xhr.responseText);
			sslCerts=resp["data"];
			updateCertsList(sslCerts);
		}
	}

	xhr.open("GET","/sslconfig",true);
	xhr.send();
}

function deleteCertBySerialNum(serial){

	console.log("delete cert by serial num");

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4 && xhr.status==200) {

			for (var i = sslCerts.length-1; i >=0;i--){

				if (sslCerts[i]["certSeqNum"]==serial){
					sslCerts.splice(i,1);
				}
			}
			tableCert.fnDeleteRow(selectedRowObject);
		}
	}

	xhr.open("DELETE","/sslconfig",true);
	xhr.send('{"serial":' + serial + '}');
}

function downloadCert(){

	console.log("??????"):
	console.log($('#publicKey').is(':checked'));
	console.log($('#privateKey').is(':checked'));
	console.log($('#pkcs12Key').is(':checked'));

	console.log("download cert by serial num " + selectedSerial);

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4 && xhr.status==200) {

			var resp = JSON.parse(xhr.responseText);

			var BB = window.Blob;
			saveAs(
				  new BB(
					  [resp["data"][0]["certPublicBody"]]
					, {type: "text/plain;charset=" + document.characterSet}
					)
				, "publicKey.crt"
			);
			saveAs(
				  new BB(
					  [resp["data"][0]["certPrivateBody"]]
					, {type: "text/plain;charset=" + document.characterSet}
					)
				, "privateKey.crt"
			);
			var byteArray = Base64Binary.decodeArrayBuffer(resp["data"][0]["certP12Body"]);
			saveAs(
				  new BB(
					  [byteArray]
					, {type: "text/plain;charset=" + document.characterSet}
					)
				, "cert.p12"
			);

		}
	}

	xhr.open("POST","/cert",true);
	xhr.send('{"serial":' + selectedSerial + '}');
}

	function initSslTable(){

		var selected = [];
    tableCert = $('#sslCerts').dataTable({
    	"iDisplayLength": 50,
    	"bLengthChange": false,
        "bPaginate": false,
        "bJQueryUI": true,
        "bRetrieve": true,
        "bDestroy" : true,
    	"aaData"   : [],
    	"data":sslCerts,
		aoColumns : 
		[
			{"sWidth": "10%"},
			{"sWidth": "7%"},
			{"sWidth": "25%"},
			{"sWidth": "13%"},
			{"sWidth": "13%"},
			{"sWidth": "9%" },
            {"sWidth": "13%"},
			{
				"sWidth"        : "10%",
				"orderable"     : false,
				"data":           null,
          		"defaultContent": '',
			  	"mRender"       : function (dataIn, type, full) {
			  		if (dataIn[1]==true)
			  		{
						return '<input type=\"button\" value=\"generate cert\" class=\"generateCertFromCa\" />';
                    }
                }
			},
			{
				"sWidth"        : "10%",
				"orderable"     : false,
				"data"          : null ,
				"defaultContent": '',
				"mRender"       : function(dataIn,type,full){
					return '<input type=\"button\" value=\"download\" class=\"downloadCertClass\" /><input style=\'float:right;\' class=\"deleteCert\" type=\"button\" value=\"\"/>';
				}
			},
			{ "sWidth": "10%","visible":false},
			{ "sWidth": "10%","visible":false},
	    ],
		"fnRowCallback": function( nRow, aData ) {
			var obj = String(aData);
			var splitted = obj.split(",");
			var $nRow = $(nRow); // cache the row wrapped up in jQuery
			
			if (splitted[13] == 1) {
				$nRow.css({"background-color":"#feafaf"})
			}
			else if (splitted[13] == 2) {
				$nRow.css({"background-color":"#F3F781"})
			}
			
			return nRow
		},
	    "order": [[0, 'dsc']]
    }).rowGrouping(
		    {
                bExpandableGrouping   : true ,
                bExpandSingleGroup    : false,
                iExpandGroupOffset    : -1   ,
                asExpandedGroups      : [""] ,
                iGroupingColumnIndex  : caFilterIndex,
				bSetGroupingClassOnTR : true,
				sGroupingClass        : "certGroupClass",
        	}
    );

		gridRowCount();
	}

function gridRowCount() {

	$('span.rowCount-grid').remove();
	$('input.expandedOrCollapsedGroup').remove();

	$('.dataTables_wrapper').find('[id|=group-id]').each(
		function () {
			var rowCount = $(this).nextUntil('[id|=group-id]').length;
			$(this).find('td').append($('<span />', { 'class': 'rowCount-grid' }).append($('<b />', { 'text': rowCount })));
		}
	);

	$('.dataTables_wrapper').find('.dataTables_filter').append($('<input />', { 'type': 'button', 'class': 'expandedOrCollapsedGroup collapsed', 'value': 'Expand All Group' }));

	$('.expandedOrCollapsedGroup').off('click');

	$('.expandedOrCollapsedGroup').on('click', function () {

		if ($(this).hasClass('collapsed')) {
		    $(this).addClass('expanded').removeClass('collapsed').val('Collapse All Group').parents('.dataTables_wrapper').find('.collapsed-group').trigger('click');
		}
		else {
		    $(this).addClass('collapsed').removeClass('expanded').val('Expand All Group').parents('.dataTables_wrapper').find('.expanded-group').trigger('click');
		}

	});
}
function reloadTable() {

	tableCert.fnClearTable();
	tableCert.fnDestroy();
	var sslCertsTemp = sslCerts;
	sslCerts=[];
	initSslTable();
	updateCertsList(sslCertsTemp);
	sslCerts=sslCertsTemp;
}

function updateCertsList(sslCertList){

	var oSettings = tableCert.fnSettings();

	tableCert.fnClearTable(this);

	for (var i = 0;i < sslCertList.length;i++){

		var serial = "";

		if (sslCertList[i]["certSignBySerial"]==-1){

			serial="self-signed";
		}
		else{

			serial="signed with " +sslCertList[i]["certSignBySerial"];
		}

		tableCert.oApi._fnAddData(oSettings,
			[
				serial                                      ,
				sslCertList[i]["certisCa"]                  ,
				sslCertList[i]["certCommonName"]            ,
				formatDate(sslCertList[i]["certStartDate"]) ,
				formatDate(sslCertList[i]["certEndDate"])   ,
				sslCertList[i]["certSeqNum"]                ,
				formatDate(sslCertList[i]["certRecordDate"]),
				"",
				"",
				"",
				""
			]
			);
	}
	
	oSettings.aiDisplay = oSettings.aiDisplayMaster.slice();
	tableCert.fnDraw();
	tableCert.oApi._fnProcessingDisplay(oSettings, false);

	var count = 1;
	var offsetGenerate=9;
	var offsetDownload=10;

	$('#sslCerts tbody tr td').on('click','.deleteCert',function(){

		selectedRowObject=tableCert.fnGetPosition( $(this).closest('tr')[0] );
		$( "#deleteDialog" ).dialog( "open" );
	});

	$('#sslCerts tbody tr td').on('click','.downloadCertClass',function(){
		$( "#downloadCertDialoo" ).dialog( "open" );
	});

	$('#sslCerts tbody tr td').on('click','.generateCertFromCa',function(){

		$( "#generateCertFromCADialog" ).dialog( "open" );
	});

	$('#sslCerts tbody tr').on('click', function () {

		if ( $(this).hasClass('row_selected') && !$(this).hasClass('certGroupClass')) {

			tableCert.$('tr.selected').removeClass('row_selected');
			$(this).removeClass('row_selected');
		}
		else if ( !$(this).hasClass('certGroupClass')){

			tableCert.$('tr').removeClass('row_selected');

			var rowIndex = tableCert.fnGetPosition( $(this).closest('tr')[0] );
			var aData = tableCert.fnGetData(rowIndex);
			selectedCert=aData;

			if (aData != null){

				selectedSerial=aData[5];
				selectedIsCa=aData[2];
			}

			$(this).removeClass('row_selected');
			$(this).addClass('row_selected');
		}
	});

	$('.expandedOrCollapsedGroup').trigger('click');
}

function initGenerateCa() {

	console.log("Pre init generating CA certificate");
	$( "#generateCertDialog" ).dialog( "open" );
}

function generateFromCA(serial){

	var timeFromVar = "";
	var timeFromTab = document.getElementById("Cert-single-input").value.split(":");
	
	if (timeFromTab[0].length==1)
		timeFromVar+= "0"+timeFromTab[0];
	else
		timeFromVar+=timeFromTab[0];

	timeFromVar+=":";

	if (timeFromTab[1].length==1)
		timeFromVar+= "0"+timeFromTab[1];
	else
		timeFromVar+=timeFromTab[1];


	var timeTo2="";
	var timeToTab2=document.getElementById("Cert-single-input2").value.split(":");

	if (timeToTab2[0].length==1)
		timeTo2+= "0"+timeToTab2[0];
	else
		timeTo2+=timeToTab2[0];

	timeTo2+=":";

	if (timeToTab2[1].length==1)
		timeTo2+= "0"+timeToTab2[1];
	else
		timeTo2+=timeToTab2[1];

	var pattern = /(\d{2})\/(\d{2})\/(\d{4})/;
	var dateTmp1 = String(String(document.getElementById("CertDatepicker1").value).replace(pattern,'$3-$1-$2') + " " + timeFromVar +":00");
	var dateTmp2 = String(String(document.getElementById("CertDatepicker2").value).replace(pattern,'$3-$1-$2') + " " + timeTo2 +":00");

	var today = new Date();
	var date1 = new Date(dateTmp1);
	var date2 = new Date(dateTmp2);

	var millisStart =0;
	var millisEnd=0;

	console.log("Generating cert from CA");

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4  && xhr.status==200) {

			var resp = JSON.parse(xhr.responseText);
			response_serial=resp["serial"];

			var generatedCert={ "certCommonName" : document.getElementById("generateFromCaCommonName").value, "certEndDate" : date2.getTime(), "certRecordDate" : new Date().getTime(), "certSeqNum" : response_serial, "certSignBySerial" : serial, "certStartDate" : date1.getTime() , "certisCa" : false };
			sslCerts.push(generatedCert);
			reloadTable();

		}
	}

	var commonName=document.getElementById("generateFromCaCommonName").value;
	var countryName=document.getElementById("generateFromCaCountryName").value;
	var provinceName=document.getElementById("generateFromCaProvinceName").value;
	var localityName=document.getElementById("generateFromCaLocalityName").value;
	var organizationName=document.getElementById("generateFromCaOrganizationName").value;
	var organizationalUnitName=document.getElementById("generateFromCaOrganizationalUnitName").value;

	xhr.open("POST","/generateCA",true);
	xhr.send('{"start_date":' + date1.getTime() + 
			',"end_date":' + date2.getTime() + 
			',"common_name":"' + commonName + 
			'","country_name":"' + countryName + 
			'","province_name":"' + provinceName + 
			'","locality_name":"' + localityName + 
			'","organization_name":"' + organizationName + 
			'","organizational_unit_name":"' + organizationalUnitName + 
			'","isCA":false,"CAserial":"' + serial + '"}');
}

function generateCA(){

	var timeFromVar = "";
	var timeFromTab = document.getElementById("caCert-single-input").value.split(":");
	
	if (timeFromTab[0].length==1)
		timeFromVar+= "0"+timeFromTab[0];
	else
		timeFromVar+=timeFromTab[0];

	timeFromVar+=":";

	if (timeFromTab[1].length==1)
		timeFromVar+= "0"+timeFromTab[1];
	else
		timeFromVar+=timeFromTab[1];


	var timeTo2="";
	var timeToTab2=document.getElementById("caCert-single-input2").value.split(":");

	if (timeToTab2[0].length==1)
		timeTo2+= "0"+timeToTab2[0];
	else
		timeTo2+=timeToTab2[0];

	timeTo2+=":";

	if (timeToTab2[1].length==1)
		timeTo2+= "0"+timeToTab2[1];
	else
		timeTo2+=timeToTab2[1];

	var pattern = /(\d{2})\/(\d{2})\/(\d{4})/;
	var dateTmp1 = String(String(document.getElementById("caCertDatepicker1").value).replace(pattern,'$3-$1-$2') + " " + timeFromVar +":00");
	var dateTmp2 = String(String(document.getElementById("caCertDatepicker2").value).replace(pattern,'$3-$1-$2') + " " + timeTo2 +":00");

	var today = new Date();
	var date1 = new Date(dateTmp1);
	var date2 = new Date(dateTmp2);

	console.log("Generating new CA cert");

	var xhr = getXHR();

	xhr.onreadystatechange=function() {

		if (xhr.readyState==4  && xhr.status==200) {
			
			var resp = JSON.parse(xhr.responseText);
			response_serial=resp["serial"];

			var generatedCert={ "certCommonName" : document.getElementById("generateCaCommonName").value, "certEndDate" : date2.getTime(), "certRecordDate" : new Date().getTime(), "certSeqNum" : response_serial, "certSignBySerial" : -1, "certStartDate" : date1.getTime() , "certisCa" : true };
			sslCerts.push(generatedCert);
			reloadTable();
		}
	}

	var commonName=document.getElementById("generateCaCommonName").value;
	var countryName=document.getElementById("generateCaCountryName").value;
	var provinceName=document.getElementById("generateCaProvinceName").value;
	var localityName=document.getElementById("generateCaLocalityName").value;
	var organizationName=document.getElementById("generateCaOrganizationName").value;
	var organizationalUnitName=document.getElementById("generateCaOrganizationalUnitName").value;

	xhr.open("POST","/generateCA",true);
	xhr.send('{"start_date":' + date1.getTime() + 
				',"end_date":' + date2.getTime() + 
				',"common_name":"' + commonName + 
				'","country_name":"' + countryName + 
				'","province_name":"' + provinceName + 
				'","locality_name":"' + localityName + 
				'","organization_name":"' + organizationName + 
				'","organizational_unit_name":"' + organizationalUnitName + '","isCA":true}');
}