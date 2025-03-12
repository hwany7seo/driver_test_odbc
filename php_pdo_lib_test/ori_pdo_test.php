<?php
require_once 'pdo_test.inc';

$options = getopt("c:");
$test_count = 50000;
if (isset($options['c'])) {
    $test_count = $options['c'];
}

$pdo = PDOTest::factory();

if (!$pdo) {
    exit("Connection Failed: " . $pdo);
}
error_log("Connected successfully");

$pdo->exec('drop table if exists test_table');
$createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
$pdo->exec($createTableSQL);
error_log("Table created successfully");

$startTime = microtime(true);
$pdo->beginTransaction();
$insert_count = 0;
$batchSize = 1000;

try {
    for ($i = 1; $i <= $test_count; $i++) {
        $name = 'oripdo' . $i;
        $insertSQL = "INSERT INTO test_table (id, name) VALUES ($i, '$name')";
        $pdo->exec($insertSQL);
        $insert_count++;

        if ($i % $batchSize == 0) {
            $pdo->commit();
            $pdo->beginTransaction();
        }
    }
    $pdo->commit();
} catch (Exception $e) {
    $pdo->rollBack();
    error_log("Error inserting data: " . $e->getMessage());
}
$endTime = microtime(true);
$elapsed_time = $endTime - $startTime;
error_log ("data inserted insert_count : " . $insert_count . " ( elapsed time: " . $elapsed_time . "s)");

$verifyInsertSQL = "SELECT COUNT(*) AS cnt FROM test_table";
$stmt = $pdo->query($verifyInsertSQL);
$row = $stmt->fetch(PDO::FETCH_ASSOC);
error_log("Data count after insert: " . $row['cnt']);

$startTime = microtime(true);
$selectSQL = "SELECT * FROM test_table";
$stmt = $pdo->query($selectSQL);

$rowCount = 0;
while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
    $rowCount++;
}
$endTime = microtime(true);
$elapsed_time = $endTime - $startTime;
error_log("data selected. rowCount : " . $rowCount . " (elapsed_time: " . $elapsed_time . "s)");
//var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>