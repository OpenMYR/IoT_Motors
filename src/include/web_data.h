#ifndef WEB_DATA_H
#define WEB_DATA_H

#define FAVICON_LEN 318
#define CONFIG_PAGE_LEN 2304
#define REDIR_LEN 23
#define OKAY_LEN 56
#define CSS_OKAY_LEN 55
#define FAV_OKAY_LEN 44
#define DEFAULT_CSS_LEN 3694

static unsigned char favicon_ico[] = {
	'H', 'T', 'T', 'P', '/', '1', '.', '1', ' ', '2', '0', '0', ' ', 'O', 'K', '\n', 
	'C', 'o', 'n', 't', 'e', 'n', 't', '-', 'T', 'y', 'p', 'e', ':', ' ', 'i', 'm', 'a', 'g', 'e', '/', 'x', '-', 'i', 'c', 'o', 'n', '\n', '\n',
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x00, 0x01, 0x00,
  0x04, 0x00, 0x28, 0x01, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x21, 0x00, 0x00, 0x2e, 0x18,
  0x00, 0x00, 0xff, 0xf1, 0xe3, 0x00, 0xe0, 0x8e, 0x36, 0x00, 0x7a, 0x3f,
  0x00, 0x00, 0xfa, 0xc7, 0x91, 0x00, 0x9e, 0x52, 0x00, 0x00, 0xfc, 0xdc,
  0xb8, 0x00, 0x57, 0x2d, 0x00, 0x00, 0xf2, 0xaa, 0x5c, 0x00, 0x1a, 0x0d,
  0x00, 0x00, 0x8f, 0x4a, 0x00, 0x00, 0xd4, 0x7a, 0x19, 0x00, 0xbd, 0x63,
  0x02, 0x00, 0x63, 0x33, 0x00, 0x00, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21,
  0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21,
  0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21,
  0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21,
  0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21,
  0x9f, 0x5c, 0x7e, 0xd4, 0xa6, 0x83, 0x0b, 0x21, 0x9f, 0x5c, 0x7e, 0xd4,
  0xa6, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char *config_page = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=UTF-8\n\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n<head>\r\n<title>Motor Control</title>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\r\n  <link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"css/default.css\" />\r\n</head>\r\n<body id=\"index_page\"> \r\n\t<div id=\"wrapper\">\r\n\t\t<div id=\"header\">\r\n        \t<h1>OpenMYR</h1>\r\n        </div>    \r\n\t\t<ul id=\"navigation\">\r\n            <li id=\"contact_us\"><a href=\"control.html\">Control</a></li>\r\n            <li id=\"about_us\"><a href=\"config.html\">Configure</a></li>\r\n            <li id=\"index\"><a href=\"index.html\">Home</a></li>\r\n        </ul>\r\n\t\t<div id=\"content\">\r\n\t\t\t<h2 class=\"right_column\">Join a new network</h2>\r\n        \t<div class=\"container\">\r\n        \t\t<form>\r\n        \t\t\tNetwork Name: <input id=\"ssid\" type=\"text\" name=\"ssid\"><br><br>\r\n        \t\t\tNetwork Password: <input id=\"pass\" type=\"password\" name=\"pass\"><br><br>\r\n        \t\t\t<input id=\"submit\" type=\"button\" value=\"Connect\" style=\"width:100px\">\r\n        \t\t</form>\r\n        \t</div>\r\n\t\t\t\r\n            <p id=\"footer\">Go to <a href=\"http://www.OpenMYR.com/\">OpenMYR.com</a> for more info</p>\r\n\t\t</div>\r\n\t</div>\r\n</body>\r\n<script type=\"text/javascript\">\r\n    document.getElementById(\"submit\").addEventListener(\"click\", function(){\r\n        var httpRequest = new XMLHttpRequest();\r\n        httpRequest.onreadystatechange=function(){\r\n            if (httpRequest.readyState==4){\r\n                if (httpRequest.status==200 || window.location.href.indexOf(\"http\")==-1){\r\n\r\n                }\r\n                else{\r\n                    alert(\"An error has occured making the request\");\r\n                }\r\n            }\r\n        }\r\n        var parameters={\r\n            code : \"C\",\r\n            ssid : document.getElementById(\"ssid\").value,\r\n            pass : document.getElementById(\"pass\").value\r\n        };\r\n        if (parameters.ssid != \"\"){\r\n            httpRequest.open(\"POST\", \"/\", true);\r\n            httpRequest.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");\r\n            httpRequest.send(JSON.stringify(parameters));\r\n        } else {\r\n            alert(\"\\\"Network Name\\\" cannot be left blank\");\r\n        }\r\n    });\r\n</script>\r\n</html>";
static char *post_redirect = "HTTP/1.1 301\nLocation:/";
static char *default_css = "HTTP/1.1 200 OK\nContent-Type: text/css; charset=UTF-8\n\n/* CSS Document */\r\n\r\n/*##############################################################\r\n  Eric Meyer's Reset.css (includes Baseline)\r\n##############################################################*/ \r\n\r\nhtml, body, div, span,\r\napplet, object, iframe,\r\nh1, h2, h3, h4, h5, h6, p, blockquote, pre,\r\na, abbr, acronym, address, big, cite, code,\r\ndel, dfn, em, font, img, ins, kbd, q, s, samp,\r\nsmall, strike, strong, sub, sup, tt, var,\r\ndd, dl, dt, li, ol, ul,\r\nfieldset, form, label, legend,\r\ntable, caption, tbody, tfoot, thead, tr, th, td {\r\n\tmargin: 0;\r\n\tpadding: 0;\r\n\tborder: 0;\r\n\tfont-weight: inherit;\r\n\tfont-style: inherit;\r\n\tfont-size: 100%;\r\n\tline-height: 1;\r\n\tfont-family: inherit;\r\n\ttext-align: left;\r\n\tvertical-align: baseline;\r\n}\r\na img, :link img, :visited img {border: 0}\r\ntable {border-collapse: collapse; border-spacing: 0}\r\nq:before, q:after,\r\nblockquote:before, blockquote:after {content: \"\"}\r\n\r\n/*##############################################################\r\n  Table Styles\r\n##############################################################*/\r\n\r\n.container {\r\n\twidth: 500px;\r\n\tpadding:5px 30px 5px 5px;\r\n\tborder: 2px solid; \r\n\tcolor : #8E8E8E;\r\n\tclear: both;\r\n}\r\n.container input {\r\n\twidth: 100%;\r\n\tclear: both;\r\n}\r\n\r\n/*##############################################################\r\n  Element Styles\r\n##############################################################*/\r\n\r\nh1 {margin-left:3px;padding-top:9px; padding-left:53px; text-shadow:1px 1px #aaa; font-size:45px; color:#f4f4f4; background:url(../images/example_logo.png) left 6px no-repeat}\r\nh3, h4, h5, h6 {margin:20px 0 15px 0; font-weight:bold; font-family:Futura; color:#c3c3c3}\r\nh2 {clear:both;margin:25px 0 15px 0; font-size:30pt; font-weight:bold; font-family:Futura, Helvetica, Arial, Verdana, sans-serif; color:#f4f4f4}\r\nh3 {font-size:22px; font-weight:normal}\r\nul, ol, dl, li {margin:0}\r\np {width:100%; text-align:left; margin:15px 0 0 0}\r\na {color:#A02121; text-decoration:underline; outline:none}\r\n\ta:hover {text-decoration:none}\r\n\t\r\n.clear {clear:both; float:left}\r\n\r\n/*##############################################################\r\n  Layout Styles\r\n##############################################################*/\r\n\r\nbody {font-size:16px; font-family:Helvetica, Arial, Verdana, sans-serif; color:#f4f4f4; background:#272727 url(../images/body_bg.jpg) 50% top repeat-x}\r\n\r\n#wrapper {width:830px; overflow:hidden; margin:0 auto; padding:0 220px}\r\n\t\r\n\t#header {float:left; display:inline; height:60px;margin:0 0 40px 5px}\r\n\t\r\n\t#navigation {float:right; display:inline; list-style-type:none; padding-left:13px; width:500px}\r\n\t\t#navigation #index { padding-top:5px; height:51px; width:146px; border-left:2px groove #c3c3c3}\r\n\t\t#navigation #about_us { padding-top:5px; height:51px; width:150px}\r\n\t\t#navigation #contact_us { padding-top:5px; height:51px; width:146px}\r\n\t\t#navigation li {float:right; margin:0; font-size:18px; border-right:2px groove #c3c3c3}\r\n\t\t\t#navigation li a {display:block; height:41px; padding-top:16px; text-align:center; color:#272727; text-decoration:none}\r\n\t\t\t\t#navigation li a:hover {color:#A12121; text-decoration:none}\r\n\t\r\n\t#content {clear:both; width:804px; padding:0 13px; background:#303030}\r\n\t\r\n\t\t#main_content {float:left; width:526px; margin:0 0 30px 0}\r\n\t\t\t\t\t\t\r\n\t\t\t.two_columns {float:left; display:inline; margin-right:30px; width:247px}\r\n\t\t\t#main_content .two_columns p {width:247px}\r\n\t\t\t#main_content p {width:526px}\r\n\t\t\t#main_content .right_column {margin:0}\r\n\t\r\n\t\t#sidebar {float:right; display:inline; width:247px; margin-left:30px}\r\n\t\t#full {float:left; width:700px; margin:0 0 30px 0}\r\n\t\t\t\t\r\n\t\t#footer {clear:both; padding:100px 0 30px 0; font-size:.85em}\r\n\t\t\r\n\t\t\r\n\t\t\r\n\t\t";

//static char *okay = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=UTF-8\n\n";

#endif