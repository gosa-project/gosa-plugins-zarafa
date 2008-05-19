

{$div}


{literal}
<script type="text/javascript">

if (typeof XMLHttpRequest != 'undefined')
{
    xmlHttpObject = new XMLHttpRequest();
}
if (!xmlHttpObject)
{
    try
    {
        xmlHttpObject = new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch(e)
    {
        try
        {
            xmlHttpObject = new ActiveXObject("Microsoft.XMLHTTP");
        }
        catch(e)
        {
            xmlHttpObject = null;
        }
    }
}

var fai_status = new Array();

function loadContent()
{
	var c = 0;

	/* Create array of available progress images once 
	 */
	if(!fai_status.length){
		for (var i = 0; i < document.images.length; i++) {
			var img = document.images[i];
			var id  = img.id;
			if(id.match(/^progress_/)){
				var mac = id.replace(/^progress_/,''); 
				mac = mac.replace(/_/g,':'); 
				fai_status[c] = new Object();
				fai_status[c]['MAC']  = mac;
				fai_status[c]['PROGRESS'] = -1;
				c ++;
			}
		}
	}

	/* Create string of macs used as parameter for getFAIstatus.php
		to retrieve all progress values.
     */
	var macs = "";
	for (var i = 0; i < fai_status.length; i++) {
		macs += fai_status[i]['MAC'] + ","
	}

	/* Send request 
     */
    xmlHttpObject.open('get','getFAIstatus.php?mac=' + macs);
    xmlHttpObject.onreadystatechange = handleContent;
    xmlHttpObject.send(null);
    return false;
}


function handleContent()
{
    if (xmlHttpObject.readyState == 4)
    {
		/* Get text and split by newline 
         */
        var text = xmlHttpObject.responseText;
		var data = text.split("\n");
		
		/* Walk through returned values and parse out 
            mac and progress value */
		for (var i = 0; i < data.length; i++) {
			var mac 	= data[i].replace(/\|.*$/,"");
			var progress= data[i].replace(/^.*\|/,"");

			// DEBUG 
			//progress = parseInt(Math.random() * 100);
	
			/* Walk through progress images and check if the 
				progress status has changed 
             */
			for (var e = 0; e < fai_status.length; e++) {

				/* */
				if(fai_status[e]["MAC"] == mac){

					/* Check if progress has changed 
					 */	
					if(fai_status[e]["PROGRESS"] != progress){
						var id 		= mac.replace(/:/g,"_"); 
						id = "progress_" + id;
						if(document.getElementById(id)){
							document.getElementById(id).src = "progress.php?x=80&y=13&p=" + progress; 
							fai_status[e]["PROGRESS"] = progress;
						}
					}
				}
			}
		}
		timer=setTimeout('loadContent()',5000);
    }
}

timer=setTimeout('loadContent()',5000);
</script>
{/literal}

