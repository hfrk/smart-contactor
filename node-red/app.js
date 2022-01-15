require('dotenv').config();

let express = require('express');
let path = require('path');
let cookieParser = require('cookie-parser');
let logger = require('morgan');
let http = require('http');
let RED = require("node-red");

let app = express();

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res, next) {
    res.status(200);
    res.sendFile(path.join(__dirname, 'public/index.html'));
});

// Create a server
let server = http.createServer(app);

// Create the settings object - see default settings.js file for other options
let settings = {
    flowFile: 'flows.json',
    flowFilePretty: true,
    adminAuth: {
        sessionExpiryTime: 86400,
        type: "credentials",
        users: [{
            username: process.env.NODE_RED_USER,
            password: process.env.NODE_RED_PASS,
            permissions: "*"
        }]
    },
    httpNodeAuth: {user: process.env.NODE_RED_USER, pass: process.env.NODE_RED_PASS},
    // httpStaticAuth: {user: process.env.NODE_RED_USER, pass: process.env.NODE_RED_PASS},
    httpAdminRoot: "/red",
    httpNodeRoot: "/api",
    userDir: path.join(__dirname, '.nodered'),
    functionGlobalContext: { }    // enables global context
};

// Initialise the runtime with a server and settings
RED.init(server,settings);

// Serve the editor UI from /red
app.use(settings.httpAdminRoot,RED.httpAdmin);

// Serve the http nodes UI from /api
app.use(settings.httpNodeRoot,RED.httpNode);

server.listen(8080);

// Start the runtime
RED.start();

module.exports = app;

