---
title: Dashboard
permalink: /dashboard/
---

<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <meta http-equiv="Access-Control-Allow-Origin" content="*">
    <title>CQS</title>

    
    <script src="https://iot.cdnedge.bluemix.net/ustatic/js/ubidots-html-canvas.min.js"></script>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/6.1.1/highcharts.js"></script>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    

    <style type="text/css" >
    /* Add your CSS code below */
    </style>

    <script type="text/javascript" >
     window.onload = function() { 
                var TOKEN = 'BBFF-Mx2qHkrUPmyGfoVPtCCW9ClVpxTVxs';
            var VARIABLE1 = '5ffa1d904763e76722b6e03d';
            var VARIABLE2 = '603b8f424763e73fc51f6e09';
            var VARIABLE3 = '604123c80ff4c363f544e8ad';

            function getDataFromVariable(variable, token, callback) {
                //var url = 'https://things.ubidots.com/api/v1.6/variables/' + variable + '/values';
		var url = 'https://industrial.api.ubidots.com/api/v1.6/variables/' + variable + '/values';
                var headers = {
                    'X-Auth-Token': token,
                    'Content-Type': 'application/json',		    
                };

                $.ajax({
                    url: url,
                    method: 'GET',		    
                    headers: headers,
		    crossDomain: true,
                    success: function (res) {
                        callback(res.results);
                    }
                });
            }

            var chart = Highcharts.chart('container', {
                chart: {
                    type: 'line'
                },
                title: {
                    text: 'CQS'
                },
                xAxis: {
                    type: 'datetime',
                },
                credits: {
                    enabled: false
                },
                tooltip: {
                    headerFormat: '<b>{series.name}: {point.y:.2f}</b><br>',
                    pointFormat: '{point.x:%Y-%m-%d %H:%M:%S} (UTC)'
                },
                series: [{
                        name: "354990401F",
                        data: []
                    },{
                        name: "354990402F",
                        data: []
                    },{
                        name: "354990403F",
                        data: []
                    }]
            });

            getDataFromVariable(VARIABLE1, TOKEN, function (values) {
                var data = values.map(function (value) {
                    collTime = value.context.collect;
                    if(collTime.indexOf("Z") === -1){
                        collTime = collTime+"Z";
                    }
                    unixtime = new Date(collTime).getTime();
                    return [unixtime, value.value];
                });

                chart.series[0].setData(data);
            });

            getDataFromVariable(VARIABLE2, TOKEN, function (values) {
                var data = values.map(function (value) {
                    collTime = value.context.collect;
                    if(collTime.indexOf("Z") === -1){
                        collTime = collTime+"Z";
                    }
                    unixtime = new Date(collTime).getTime();
                    return [unixtime, value.value];
                });

                chart.series[1].setData(data);
            });


            getDataFromVariable(VARIABLE3, TOKEN, function (values) {
                var data = values.map(function (value) {
                    collTime = value.context.collect;
                    if(collTime.indexOf("Z") === -1){
                        collTime = collTime+"Z";
                    }
                    unixtime = new Date(collTime).getTime();
                    return [unixtime, value.value];
                });

                chart.series[2].setData(data);
            });

     } 
    </script>
</head>
<body>
    <div id="container" style="min-width: 310px; height: 310px; margin: 0 auto"></div>
</body>
</html>

