<?php
$dsn = 'DSN=CUBRID Driver Unicode;UID=dba;PWD=';
$conn = odbc_connect($dsn, '', '');

try {
    if (!$conn) {
        exit("Connection Failed: " . $conn);
    }
    error_log("Connected successfully");

    odbc_exec($conn, 'DROP TABLE IF EXISTS test_table');
    error_log("Table drop successfully");
    $createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
    odbc_exec($conn, $createTableSQL);
    error_log("Table created successfully");

    $startTime = microtime(true);
    
    // Prepare the insert statement
    $insertSQL = "INSERT INTO test_table (id, name) VALUES (?, ?)";
    $stmt = odbc_prepare($conn, $insertSQL);
    
    if (!$stmt) {
        exit("Failed to prepare statement");
    }

    for ($i = 1; $i <= 50000; $i++) {
        $name = 'odbclib' . $i;
        $params = [$i, $name];
        odbc_execute($stmt, $params);
    }
    
    $endTime = microtime(true);
    $insertTime = $endTime - $startTime;
    error_log("Data inserted (elapsed time: " . $insertTime . "s)");

    $startTime = microtime(true);
    $selectSQL = "SELECT * FROM test_table";
    $result = odbc_exec($conn, $selectSQL);

    $rowCount = 0;
    while (odbc_fetch_row($result)) {
        $rowCount++;
    }
    $endTime = microtime(true);
    $selectTime = $endTime - $startTime;
    error_log("Data selected. rowCount: " . $rowCount . " elapsed_time: " . $selectTime . "s");

    odbc_close($conn);
} finally {
    error_log("End");
}
?>
