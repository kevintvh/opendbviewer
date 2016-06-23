/*
 * DatabaseController.cpp
 *
 *  Created on: 10 juin 2016
 *      Author: echopin
 */

#include "Database/DatabaseController.h"
#include "GUIController/QDatabaseSelectionViewController.h"
#include "GUI/QDatabaseConnectionView.h"
#include "GUIController/QDatabaseConnectionViewController.h"

#include <QSqlDatabase>
#include <QListView>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QSqlRecord>
#include <QStandardItem>
#include <QStringList>
#include <QTime>
#include <QSqlRecord>
#include <QSqlField>

DatabaseController::DatabaseController(const QString& szFilename)
{
	m_szFilename = szFilename;
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_db.setDatabaseName(m_szFilename);
}

DatabaseController::~DatabaseController()
{

}

bool DatabaseController::openDatabase()
{
	return m_db.open();
}

void DatabaseController::closeDataBase()
{
	m_db.close();
}

bool DatabaseController::loadTables(DbLoadTableCB func, void* user_data)
{
	if(openDatabase()){

		QList<QString> szTableList = m_db.tables();
		QList<QString>::const_iterator iter = szTableList.begin();
		while(iter != szTableList.end())
		{
			if(func){
				func(*iter, user_data);
			}
			iter++;
		}

		closeDataBase();
	}

	return true;
}

bool DatabaseController::loadSystemTables(DbLoadTableCB func, void* user_data)
{
	if(openDatabase()){

		QList<QString> szTableList = m_db.tables(QSql::SystemTables);

		QList<QString>::const_iterator iter = szTableList.begin();
		while(iter != szTableList.end())
		{
			if(func){
				func(*iter, user_data);
			}
			iter++;
		}

		closeDataBase();
	}

	return true;
}

bool DatabaseController::loadViewsTables(DbLoadTableCB func, void* user_data)
{
	if(openDatabase()){

			QList<QString> szTableList = m_db.tables(QSql::Views);
			QList<QString>::const_iterator iter = szTableList.begin();
			while(iter != szTableList.end())
			{
				if(func){
					func(*iter, user_data);
				}
				iter++;
			}

			closeDataBase();
		}
	return true;
}

bool DatabaseController::loadTableDescription(const QString& szTableName, DbLoadTableDescription func, void* user_data)
{
	openDatabase();

	QSqlQuery tableInfoQuery("PRAGMA table_info("+szTableName+");");
	tableInfoQuery.exec();
	while (tableInfoQuery.next())
    {
		QString szName = tableInfoQuery.value(1).toString();
		QString szType = tableInfoQuery.value(2).toString();
		bool bNotNull = tableInfoQuery.value(3).toBool();
		QString szDefaultValue = tableInfoQuery.value(4).toString();
		QString szPk = tableInfoQuery.value(5).toString();
		func(szName, szType, bNotNull, szDefaultValue, szPk, user_data);
    }
	m_szResultString = makeQueryResultString(tableInfoQuery);
	tableInfoQuery.finish();

	closeDataBase();

	return true;
}

bool DatabaseController::loadTableData(const QString& szTableName, const QString& szFilter, DbLoadTableData func, void* user_data)
{
	openDatabase();

	QList<QString> pRowData;

	//Get the list of column names from the table
	QStringList pColumnName = listColumnNames(szTableName);

	//Creating a string from a list
	QString columnNamesString = pColumnName.join(", ");

	//Using the string in the query
	QSqlQuery tableDataQuery;
	QString szQuery = "SELECT rowid as rowid, "+columnNamesString+" FROM "+szTableName;

	if(!szFilter.isEmpty())//If there is no filter, execute query
	{
		szQuery += " WHERE "+szFilter;
	}
	tableDataQuery.exec(szQuery);

	/*if there is no data to get, get both pColumnName and empty pRowData for setting the header,
	 * and set the position back to the first record*/
	if (tableDataQuery.next() == false)
		func(pColumnName, pRowData, user_data);
		tableDataQuery.previous();

	while(tableDataQuery.next())
	{
		int currentColumnNumber;
		for (currentColumnNumber = 0; currentColumnNumber <= pColumnName.size(); currentColumnNumber++)
		{
			pRowData << tableDataQuery.value(currentColumnNumber).toString();
		}
		func(pColumnName, pRowData, user_data);
		//Clearing pRowData to have an empty list when starting the while loop again
		pRowData.clear();
	}
	m_szResultString = makeQueryResultString(tableDataQuery);
	tableDataQuery.finish();
	closeDataBase();

	return true;
}

bool DatabaseController::loadTableCreationScript(const QString& szTableName, DbLoadTableCreationScript func, void* user_data)
{
	openDatabase();
	QString szCreationScriptString;
	if(szTableName == "sqlite_master")
		{
			func(szCreationScriptString, user_data);
		}

	QSqlQuery tableCreationScriptQuery("SELECT sql FROM sqlite_master WHERE type = 'table' AND name = '"+szTableName+"';");
	tableCreationScriptQuery.exec();

	while(tableCreationScriptQuery.next())
	{
		szCreationScriptString = tableCreationScriptQuery.value(0).toString();
		func(szCreationScriptString, user_data);
	}

	closeDataBase();

	return true;
}

bool DatabaseController::loadWorksheetQueryResults(QString& szWorksheetQuery, DbLoadWorksheetQueryResults func, void* user_data)
{
	openDatabase();

	//Creates a query from the data given in the worksheet text edit
	QSqlQuery worksheetQuery(szWorksheetQuery);
	worksheetQuery.exec();

	QList<QString> pRowData;
	QList<QString> pColumnNameList;

	int currentColumnNumber;
	//appending column names to columnNameList
	for (currentColumnNumber = 0; currentColumnNumber < worksheetQuery.record().count(); currentColumnNumber++)
	{
		QSqlField field = worksheetQuery.record().field(currentColumnNumber);
		pColumnNameList << field.name();
	}

	while(worksheetQuery.next())
	{
		int currentColumnNumber;
		for (currentColumnNumber = 0; currentColumnNumber < worksheetQuery.record().count(); currentColumnNumber++)
		{
			pRowData << worksheetQuery.value(currentColumnNumber).toString();
		}

		func(pColumnNameList, pRowData, user_data);
		//Clearing pRowData to have an empty list when starting the while loop again
		pRowData.clear();
	}



	m_szResultString = makeQueryResultString(worksheetQuery);

	closeDataBase();

	return true;
}

QString DatabaseController::makeQueryResultString(QSqlQuery query)
{
	QString szResultString("");//Creates an empty string
	QString szNumberOfRows = makeStringNumberOfRows(query);//Gets the number of lines in the query and converts it to string
	QTime time;
	//Creating the result string with query information
	szResultString.append(time.currentTime().toString()+": "+szNumberOfRows+" row(s) selected/affected \n"+query.lastQuery()+"\n\n");

	return szResultString;
}

QString DatabaseController::makeStringNumberOfRows(QSqlQuery query)
{
	//Gets the number of rows as numRowsAffected() does not seem to be working
	int numberOfRows = 0;

		if(query.last())
		{
		    numberOfRows =  query.at() + 1;
		    query.first();
		    query.previous();
		}
	//Converts the number to a string
	QString szNumberOfRows = QString::number(numberOfRows);

	return szNumberOfRows;
}

QStringList DatabaseController::listColumnNames(QString szTableName)
{
	QStringList szListColumnName;
	QSqlQuery tableInfoQuery("PRAGMA table_info("+szTableName+");");
	tableInfoQuery.exec();
	while (tableInfoQuery.next())
	   {
		QString szName = tableInfoQuery.value(1).toString();
		szListColumnName += szName;
	   }
	return szListColumnName;
}

QString DatabaseController::getQueryResultString() const
{
	return m_szResultString;
}

