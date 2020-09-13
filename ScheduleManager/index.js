var http = require("http");
var https = require("https");
var url = require('url');
var moment = require('moment');

const { ifError } = require("assert");

const myAddressHash = "5B22467660E1D550E8507FAE7B61B7C3";

// set up how often to query Republic Services for service info
//            hours * min/hr * sec/min * milli/sec
const queryInteval  = 1 * 60 * 60 * 1000;

// a data object we can stick information into
let upcomingTrashInfo = {
   currentDate: null,
   upcomingService: [],
   happeningToday: "NOTHING"
};

function retrieveNextTrashDay(){
   const options = {
      hostname: 'www.republicservices.com',
      port: 443,
      path: '/api/v1/publicPickup?siteAddressHash='+myAddressHash,
      method: 'GET',
      accept: 'application/json, */*'
   }

   let req = https.get(options,(res)=>{
      res.setEncoding('utf8');
      res.on('data', (chunk) => {
         let dataObject = JSON.parse(chunk);
         processTrashData(dataObject.data);
      });
   });

   req.on('error', (e) => {
      console.error(`problem with request: ${e.message}`);
   });
   req.end();
}

function processTrashData(trashData){
   let today = moment();
   upcomingTrashInfo.currentDate = today.format("MM/DD/YYYY");
   upcomingTrashInfo.happeningToday = "NOTHING";
   upcomingTrashInfo.upcomingService = [];

   trashData.forEach(service => {
      let nextDayForService = moment(service.nextServiceDays[0]);
      let servicetype = service.containerType === 'RC' ? "RECYCLE" : "TRASH";
      let daysTillNextService = nextDayForService.diff(today,'days');

      if(daysTillNextService === 0){
         upcomingTrashInfo.happeningToday = servicetype;
      }

      upcomingTrashInfo.upcomingService.push({serviceType: servicetype, tillNextService: daysTillNextService, nextDayForService: nextDayForService.format('MM/DD/YYYY')});
      console.debug(`${servicetype} in ${daysTillNextService} days on ${nextDayForService.format('MM/DD/YYYY')}`);
   });
}

function binResponseHandler(req, resp){
   resp.writeHead(200,{'Content-Type':'text/json'});
   resp.end(`{happeningToday:'${upcomingTrashInfo.happeningToday}'}`);
}

function humanResponseHandler(req, resp){
   resp.writeHead(200, {'Content-Type': 'text/html'});
   resp.write(`Today is ${upcomingTrashInfo.currentDate}<br/>`);
   upcomingTrashInfo.upcomingService.forEach(service =>{
      resp.write(`${service.serviceType} in ${service.tillNextService} days on ${service.nextDayForService}<br/>`);
   });
   resp.write(`Happening Today: ${upcomingTrashInfo.happeningToday}<br/>`);
   resp.end();
}

// run once immediately
retrieveNextTrashDay();
// run again every queryInterval
setInterval(retrieveNextTrashDay, queryInteval);

// now start the server
http.createServer(function (request, response) {
   let page = url.parse(request.url).pathname;
   if(page === '/'){
      binResponseHandler(request,response);
   } else if(page === '/info'){
      humanResponseHandler(request,response);
   } else{
      response.writeHead(404);
      response.end('404');
   }
}).listen(8080);

console.log('Server running at http://127.0.0.1:8080/');
