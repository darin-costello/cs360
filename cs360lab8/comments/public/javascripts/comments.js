$(document).ready(function(){
    $("#serialize").click(function(){
        var myobj = {Name:$("#Name").val(),Comment:$("#Comment").val()};
        jobj = JSON.stringify(myobj);
        $("#json").text(jobj);
	var url = "comment";
	$.ajax({
		url:url,
		type: "POST",
		data: jobj,
		contentType: "application/json; Charset=utf-8",
		success: function(data, textStatus) {
			$("#done").html(textStatus);
		}
	})
    });
	$("#getThem").click(function() {
		$.getJSON('comment', function(data) {
			console.log(data);
			var everything = "<ul>";
			for(var comment in data) {
				com = data[comment];
				everything += "<li><div class='pic'><img src='";
				everything += "ppic/" + com.Name + "'/>"
				everything += "Name: " + com.Name + " -- Comment: " + com.Comment + "</li></div>";
			}
			everything += "</ul>"
			$("#comments").html(everything);
		})
	})
	$("#addPic").click(function() {
		var myobj = {Name:$("#Namep").val(),URL:$("#URL").val()};
		jobj = JSON.stringify(myobj);
		var url = "ppic/"+$("#Namep").val();
		$.ajax({
			url:url,
			type: "POST",
			data: jobj,
			contentType: "application/json; Charset=utf-8",
			sucess: function(data, textStatus) {
				console.log("textStatus")
				}
		});
	});
				
});
