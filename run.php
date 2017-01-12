 <?php
 $result = mysql_connect("127.0.0.1", "root", "");
 $heartbeat = $_GET["heartbeat"];

 $sqlt = "insert into testdb.data values ($heartbeat)";
 echo "$sqlt";
 $ret = mysql_query($sqlt);
 
?>
