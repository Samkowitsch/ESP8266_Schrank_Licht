
var connection = new WebSocket('ws://' + location.hostname + ':81/' , ['arduino']);

var vLed_Mode , vHelligkeit ;

connection.onopen = function(){
    connection.send('Connect : ' + new Date());
    document.getElementById("but1").innerText = "Connection Open";
};

connection.onerror = function(error){
    console.log('Websocket Error', error);
};

connection.onmessage = function(e){
    console.log('Server : ' , e.data);
};

connection.onclose = function(){
    console.log('Websocket connection closed');
    document.getElementById("but1").innerText = "Connection Closed";
};

function checkConnection(){
    var connectionState;
    connectionState = connection.readyState; //0 = Opening , 1 = Open , 2 = Closing, 3 = Closed
    if(connectionState == 3){
        location.reload();
    }

}




function Led_Mode(Mode){
    checkConnection();
    vLed_Mode = Mode;
    var message = "Mode:" + Mode;
    connection.send(message);
}

function Helligkeit(h){
    checkConnection();
    vHelligkeit = h;
    var message = "Helligkeit:" + h;
    connection.send(message);
}


function rgbMouseOver(){
    document.getElementById("RGBdropDown").style.display = "flex";

}

function rgbMouseOut(){
    document.getElementById("RGBdropDown").style.display = "none";

}
