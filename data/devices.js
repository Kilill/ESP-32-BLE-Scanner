function sendit() {
	$.ajax({
            url: 'rest/status.json',
            type: 'POST',
            data: JSON.stringify(Status),
            contentType: 'application/json; charset=utf-8',
            dataType: 'json',
            async: false,
            success: function(msg) {
                alert(msg);
            }
        });
}

function foo (){
	// JQuery.getjSON("devices.json");
	var row;
	var tr;


	for (let i=1;i<10;i++) {
		row=$("<tr>").attr({id:"r"+i});
		row.appendTo($("#devices"));
		td=$("<td>").attr({id:"du"+i}).appendTo('#r'+i);
		$("<input>").attr({type:'text', class:"uuid",id:'uuid'+i}).val('213-123 :'+i).appendTo('#du'+i);
		td=$("<td>").attr({id:"dn"+i}).appendTo('#r'+i);
		$("<input>").attr({type:'text', class:"name",id:'name'+i}).val('a phone :'+i).appendTo('#dn'+i);
	}

}
console.log("loading");
$(document).ready(foo);
