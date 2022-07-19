
function getdevices (){
	var row;
	var tr;
	var data;
	var i;

	$.getJSON("devices.json",data, function(data) {
		console.log(data);
		devices=data.devices;
		console.log(devices);

		i=0;
		devices.forEach(function(device ){
			row=$("<tr>").attr({id:"r"+i});
			row.appendTo($("#devices"));
			td=$("<td>").attr({id:"du"+i}).appendTo('#r'+i);
			$("<input>").attr({type:'text', class:"uuid",id:'uuid'+i}).val(device.uuid).appendTo('#du'+i);
			td=$("<td>").attr({id:"dn"+i}).appendTo('#r'+i);
			$("<input>").attr({type:'text', class:"name",id:'name'+i}).val(device.name).appendTo('#dn'+i);
			i++;
		});
	});

}
//$('#devForm').submit(function(e) {
$(document).on('click', '#devSubmit', function(e) {

	e.preventDefault();
  	var headersText = [];
  	var $headers = $("th");
	var myRows=[];
	var devices={};
  // Loop through grabbing everything
  	var $rows = $("tbody tr").each(function(index) {
    	$cells = $(this).find("td");
    	myRows[index] = {};

    	$cells.each(function(cellIndex) {
      	// Set the header text
      		if(headersText[cellIndex] === undefined) {
				headersText[cellIndex] = $($headers[cellIndex]).text();
			}
			// Update the row object with the header/cell combo
			myRows[index][headersText[cellIndex]] = $(this).find("input").val();
		});    
	});
	devices={'devices': myRows};
	jsondata=JSON.stringify(devices);
	console.log(jsondata);
	$.post('/devices',jsondata,function(result) {
		console.log("success",result);
	});
});

$(document).ready(getdevices);
