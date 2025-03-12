<?php
require_once 'connect.inc';
$conn = cubrid_connect($host, $port, $db, $user, $passwd);

$options = getopt("c:");
$test_count = 50000;
if (isset($options['c'])) {
    $test_count = $options['c'];
}

if (!$conn) {
    exit("Connection Failed: " . $conn);
}
error_log("Connected successfully");

cubrid_execute($conn, 'drop table if exists test_table');

$createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
cubrid_execute($conn, $createTableSQL);
error_log("Table created successfully");

$startTime = microtime(true);
$insert_count = 0;
$batchSize = 1000;

try {
    $insertSQL = "INSERT INTO test_table (id, name) VALUES (?, ?)";
    $req = cubrid_prepare($conn, $insertSQL);
    for ($i = 1; $i <= $test_count; $i++) {
        $name = 'oriPHP' . $i;
        cubrid_bind($req, 1,  $i);
        cubrid_bind($req, 2, $name);
        cubrid_execute($req);           
    }

} catch (Exception $e) {
    $pdo->rollBack();
    error_log("Error inserting data: " . $e->getMessage());
}
$endTime = microtime(true);
$elapsed_time = $endTime - $startTime;
error_log ("data inserted. ( elapsed time: " . $elapsed_time . "s)");

$startTime = microtime(true);
$selectSQL = "SELECT * FROM test_table";
$req = cubrid_execute($conn, $selectSQL);

$rowCount = 0;
while ($row = cubrid_fetch_assoc($req)) {
    $rowCount++;
}
$endTime = microtime(true);
$elapsed_time = $endTime - $startTime;
error_log("data selected. rowCount : " . $rowCount . " (elapsed_time: " . $elapsed_time . "s)");
//var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>