#!/usr/bin/python

import os

print("Content-Type: text/html\n")

query_string = os.environ.get("QUERY_STRING", "")

params = {}
if query_string:
    pairs = query_string.split("&")
    for pair in pairs:
        key, value = pair.split("=")
        params[key] = value

print("<html>")
print("<head>")
print("<title>GET Method</title>")
print("</head>")
print("<link rel=\"stylesheet\" type=\"text/css\" href=\"/cgi-bin/css/style.css\">")
print("<body>")
print("<div class=\"container\">")
print("<h1>GET Method</h1>")
print("<p>Your Text: {}</p>".format(params.get("get_param", "")))
print("<div class=\"hamster\">")
print("<img src=\"/cgi-bin/img/hamster.jpg\" alt=\"hamster\">")
print("</div>")
print("</div>")
print("</body>")
print("</html>")