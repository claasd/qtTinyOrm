
#include "Parser.h"
#include "SqlWriter.h"
#include "ClassWriter.h"

#include <iostream>
#include <exception>

#include <QDir>

int main(int argc, char* argv[])
{
	try
	{
		Parser p;
		p.Parse("test.xml");
		const Database& db = p.getDatabase();
		SqlWriter sqlWriter(db);
        QDir current(".");
        current.mkdir("output");
        current.cd("output");
        sqlWriter.Write("output/sqlite.sql",SqlWriter::SqLiteType);
        sqlWriter.Write("output/mysql.sql", SqlWriter::MysqlType);

		ClassWriter classWriter(db);

		current.mkdir("cpp");
        classWriter.Write("output/cpp");

		return 0;
	}
	catch(const std::exception& e)
	{
		std::cout << e.what();
	}
	std::cin.get();

}
		 
