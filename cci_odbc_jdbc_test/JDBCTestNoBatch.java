import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class JDBCTestNoBatch {

    private static final String URL = "jdbc:CUBRID:localhost:33000:demodb:dba::";
    private static final int DATA_COUNT = 50000;
    private static final String USER = "dba";
    private static final String PASSWORD = "";
    
    public static void main(String[] args) {
        try {
            Class.forName("cubrid.jdbc.driver.CUBRIDDriver");
        } catch(Exception e){
            e.printStackTrace();
        }

        try (Connection conn = DriverManager.getConnection(URL, USER, PASSWORD)) {
            System.out.println("Connected successfully");

            try (Statement stmt = conn.createStatement()) {
                stmt.execute("DROP TABLE IF EXISTS test_table");
                System.out.println("Table dropped successfully");

                String createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
                stmt.execute(createTableSQL);
                System.out.println("Table created successfully");

                String insertSQL = "INSERT INTO test_table (id, name) VALUES (?, ?)";
                try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                    long startTime = System.currentTimeMillis();

                    for (int i = 1; i <= DATA_COUNT; i++) {
                        pstmt.setInt(1, i);
                        pstmt.setString(2, "jdbcdt" + i);
                        pstmt.executeUpdate();
                    }

                    long endTime = System.currentTimeMillis();
                    System.out.println("Data inserted (elapsed time: " + (endTime - startTime) + "ms)");
                }

                String SQL = "SELECT COUNT(*) FROM test_table";
                ResultSet rst = stmt.executeQuery(SQL);
                while (rst.next()) {
                    int rowcount = rst.getInt(1);
                    System.out.println("rowcount : " + rowcount);
                }
                rst.close();

                long selectStartTime = System.currentTimeMillis();
                String selectSQL = "SELECT * FROM test_table";
                try (ResultSet rs = stmt.executeQuery(selectSQL)) {
                    int rowCount = 0;
                    while (rs.next()) {
                        rowCount++;
                    }
                    long selectEndTime = System.currentTimeMillis();
                    System.out.println("Data selected. rowCount: " + rowCount + " (elapsed time: " + (selectEndTime - selectStartTime) + "ms)");
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}
