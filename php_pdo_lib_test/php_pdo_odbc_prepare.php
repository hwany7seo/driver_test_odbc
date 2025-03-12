<?php
$dsn = 'odbc:Driver=CUBRID Driver Unicode;db_name=demodb;server=192.168.2.33;port=33000';
$username = 'dba';
$password = '';

try {
    $options = getopt("c:");
    $test_count = 100;
    if (isset($options['c'])) {
        $test_count = $options['c'];
    }
    
    $pdo = new PDO($dsn, $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    error_log("Connected successfully");

    $pdo->exec('DROP TABLE IF EXISTS test_table');
    $createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
    $pdo->exec($createTableSQL);
    error_log("Table created successfully");

    $startTime = microtime(true);
    $insert_count = 0;
    $batchSize = 1000;

    $insertSQL = "INSERT INTO test_table (id, name) VALUES (:id, :name)";
    $stmt = $pdo->prepare($insertSQL);

    for ($i = 1; $i <= $test_count; $i++) {
        $id = $i;
        $name = 'preodb' . $i;
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->bindParam(':name', $name, PDO::PARAM_STR);
        $stmt->execute();
    }

    $endTime = microtime(true);
    $elapsed_time = $endTime - $startTime;
    error_log ("data inserted (elapsed time: " . $elapsed_time . "s)");

    $startTime = microtime(true);
    $selectSQL = "SELECT * FROM test_table";
    $stmt = $pdo->prepare($selectSQL);
    $stmt->execute();

    $rowCount = 0;
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $rowCount++;
    }

    $endTime = microtime(true);
    $elapsed_time = $endTime - $startTime;
    error_log("data selected. rowCount : " . $rowCount . " elapsed_time: " . $elapsed_time . "s)");

} catch (PDOException $e) {
    error_log("Connection failed: " . $e->getMessage());
}
?>
