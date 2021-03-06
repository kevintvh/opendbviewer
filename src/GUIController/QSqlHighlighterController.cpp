/*
 * QSqlHighlighterController.cpp
 *
 *  Created on: 1 juil. 2016
 *      Author: echopin
 */

#include "GUIController/QSqlHighlighterController.h"

QSqlHighlighterController::QSqlHighlighterController(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	//keyword format
	m_keywordFormat.setForeground(Qt::darkRed);
	m_keywordFormat.setFontWeight(QFont::Bold);
	m_keywordFormat.setFontCapitalization(QFont::AllUppercase);//not working
	QStringList szKeywordPatterns;

	szKeywordPatterns = makeKeywordList();
	foreach(const QString &szPattern, szKeywordPatterns)
	{
		rule.m_pattern = QRegExp(szPattern);
		rule.m_format = m_keywordFormat;
		highlightingRules.append(rule);
	}

	//single line comment format rule
	m_singleLineCommentFormat.setForeground(Qt::blue);
	rule.m_pattern = QRegExp("--[^\n]*");
	rule.m_format = m_singleLineCommentFormat;
	highlightingRules.append(rule);

	//multiline comment format rule used in mySQL
	m_multiLineCommentFormat.setForeground(Qt::blue);
	m_commentStartExpression = QRegExp("/\\*");
	m_commentEndExpression = QRegExp("\\*/");

	//quotation format rule
	//TODO multiline quotation format, could not get it to work alongside multiline comment format
	m_quotationFormat.setForeground(Qt::magenta);
	rule.m_pattern = QRegExp("'.*'");
	rule.m_format = m_quotationFormat;
	highlightingRules.append(rule);
}

QSqlHighlighterController::~QSqlHighlighterController()
{
}

void QSqlHighlighterController::highlightBlock(const QString &szText)
{
	foreach (const HighlightingRule &rule, highlightingRules)
	{
		QRegExp expression(rule.m_pattern);
		expression.setCaseSensitivity(Qt::CaseInsensitive);
		int index = expression.indexIn(szText);
		while (index >= 0)
		{
			int length = expression.matchedLength();
			setFormat(index, length, rule.m_format);
			index = expression.indexIn(szText, index + length);
	    }
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = m_commentStartExpression.indexIn(szText);

	while (startIndex >= 0) {
		int endIndex = m_commentEndExpression.indexIn(szText, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = szText.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex + m_commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, m_multiLineCommentFormat);
		startIndex = m_commentStartExpression.indexIn(szText, startIndex + commentLength);
	}
}

QStringList QSqlHighlighterController::makeKeywordList()
{
	//Defining the list of keywords here, could maybe be defined in a text file
	QStringList szKeywordList;
	//Words are sorted alphabetically
	szKeywordList 	<< "\\bABORT\\b" << "\\bACTION\\b" << "\\bADD\\b" << "\\bAFTER\\b" << "\\bALL\\b" << "\\bALTER\\b"
					<< "\\bANALYZE\\b" << "\\bAND\\b" << "\\bAS\\b" << "\\bASC\\b" << "\\bATTACH\\b" << "\\bAUTOINCREMENT\\b" << "\\bBEFORE\\b"
					<< "\\bBEGIN\\b" << "\\bBETWEEN\\b" << "\\bBY\\b" << "\\bCASCADE\\b" << "\\bCASE\\b" << "\\bCAST\\b" << "\\bCHECK\\b" << "\\bCOLLATE\\b"
					<< "\\bCOLUMN\\b" << "\\bCOMMIT\\b" << "\\bCONFLICT\\b" << "\\bCONSTRAINT\\b" << "\\bCREATE\\b" << "\\bCROSS\\b"
					<< "\\bCURRENT_DATE\\b" << "\\bCURRENT_TIME\\b" << "\\bCURRENT_TIMESTAMP\\b" << "\\bDATABASE\\b"
					<< "\\bDEFAULT\\b" << "\\bDEFERRABLE\\b" << "\\bDEFERRED\\b" << "\\bDELETE\\b" << "\\bDESC\\b" << "\\bDETACH\\b"
					<< "\\bDISTINCT\\b" << "\\bDROP\\b" << "\\bEACH\\b" << "\\bELSE\\b" << "\\bEND\\b" << "\\bESCAPE\\b" << "\\bEXCEPT\\b"
					<< "\\bEXCLUSIVE\\b" << "\\bEXISTS\\b" << "\\bEXPLAIN\\b" << "\\bFAIL\\b" << "\\bFOR\\b" << "\\bFOREIGN\\b" << "\\bFROM\\b"
					<< "\\bFULL\\b" << "\\bGLOB\\b" << "\\bGROUP\\b" << "\\bHAVING\\b" << "\\bIF\\b" << "\\bIGNORE\\b" << "\\bIMMEDIATE\\b" << "\\bIN\\b"
					<< "\\bINDEX\\b" << "\\bINDEXED" << "INITIALLY" << "INNER" << "INSERT" << "INSTEAD\\b"
					<< "\\bINTERSECT\\b" << "\\bINTO\\b" << "\\bIS\\b" << "\\bISNULL\\b" << "\\bJOIN\\b" << "\\bKEY\\b" << "\\bLEFT\\b" << "\\bLIKE\\b"
					<< "\\bLIMIT\\b" << "\\bMATCH" << "NATURAL" << "NO" << "NOT" << "NOTNULL" << "NULL" << "OF\\b"
					<< "\\bOFFSET\\b" << "\\bON" << "OR" << "ORDER" << "OUTER" << "PLAN" << "PRAGMA" << "PRIMARY\\b"
					<< "\\bQUERY\\b" << "\\bRAISE" << "RECURSIVE" << "REFERENCES" << "REGEXP" << "REINDEX\\b"
					<< "\\bRELEASE\\b" << "\\bRENAME" << "REPLACE" << "RESTRICT" << "RIGHT" << "ROLLBACK\\b" << "\\bROW\\b"
					<< "\\bSAVEPOINT\\b" << "\\bSELECT\\b" << "\\bSET\\b" << "\\bTABLE\\b" << "\\bTEMP\\b" << "\\bTEMPORARY\\b" << "\\bTHEN\\b"
					<< "\\bTO\\b" << "\\bTRANSACTION\\b" << "\\bTRIGGER\\b" << "\\bUNION\\b" << "\\bUNIQUE\\b" << "\\bUPDATE\\b" << "\\bUSING\\b"
					<< "\\bVACUUM\\b" << "\\bVALUES\\b" << "\\bVIEW\\b" << "\\bVIRTUAL\\b" << "\\bWHEN\\b" << "\\bWHERE\\b" << "\\bWITH\\b"
					<< "\\bWITHOUT";
	szKeywordList 	<< "\\bTEXT\\b" << "\\bINTEGER\\b" << "\\bREAL\\b" << "\\bNUMERIC\\b" << "\\bNONE\\b" << "\\bBLOB\\b";
	szKeywordList 	<< "\\bTRUE\\b" << "\\bFALSE\\b";

	return szKeywordList;
}
