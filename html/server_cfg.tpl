<html><head><title>Server Config</title>
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>The Server</h1>
<p>
Server is now running on port %httpdPort%. You can update that using the form below.
</p>
<form method="post" action="server_cfg.cgi">
<input type="input" name="port" value="%httpdPort%">
<input type="submit" value="Apply">
</form>
</div>
</body></html>
