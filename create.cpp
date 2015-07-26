/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

#include <string.h>
#include <sstream>
#include <stdio.h>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "fcgio.h"

using namespace std;

int main(void)
{
	streambuf * cin_streambuf = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);
	
	while(FCGX_Accept_r(&request) == 0){
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);
		
		string queryString;
		
		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);
		
		cout << "Content-type: text/plain\r\n\r\n";
		
		queryString = FCGX_GetParam("QUERY_STRING", request.envp);

		cout << "QS: " << queryString << endl;
		
		unsigned short qsLen = queryString.length();
		char * input = new char[qsLen];
		queryString.copy(input, qsLen);
		
		string userN = strtok(input, "+");
		string passH = strtok(NULL, "+");
		
		cout << "\nPARAMS: \n\tUSER: " << userN << 
				"\n\tPASS: " << passH << endl;	

		delete input;

		try {
			sql::Driver *driver;
			sql::Connection *con;
			sql::Statement *stmt;
			sql::ResultSet *res;
			
			cout << "\nConnect to localhost:" << endl;
			cout << "\tCheck for: \n\t\tqueried username '" << userN << "'";
			
			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect("localhost", "root", "root");
			//~ /* Connect to the MySQL test database */
			con->setSchema("accounts");
			stmt = con->createStatement();
			stmt->execute("USE accounts");
			
			stringstream stmtvar;
			stmtvar << "SELECT id FROM main WHERE id = '" << userN << "'";
			cout << "\n\t\t" << stmtvar.str();
			res = stmt->executeQuery(stmtvar.str());
			
			bool hasResult = false;
			while (res->next()){
				if (res->getString("id") == userN){
					cout << "\n\n\t\tAccount exists!";
					hasResult = true;
				}
			}
			
			if(!hasResult){
				cout << "\n\n\t\tAccount does not exist!";
			
				stringstream stmtvar2;
				stmtvar2 << "INSERT INTO main(id, passhash) VALUES ('" << 
							userN << "','" << passH << "')";
				cout << "\n" << stmtvar2.str();
				stmt->executeUpdate(stmtvar2.str());
				
				cout << "\n\n\t\tAccount created!";
			}
				
			
			delete stmt;
			delete res;
			delete con;
		} catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		}//end sql
	}//end fcgi

	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);

	return EXIT_SUCCESS;
}
