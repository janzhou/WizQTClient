#include "wizxmlrpc.h"
#include <QUrl>


bool WizXmlRpcValueFromXml(CWizXMLNode& nodeValue, CWizXmlRpcValue** ppRet);

bool WizXmlRpcParamsToXml(CWizXMLDocument& doc, const CString& strMethodName, \
                          CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2, \
                          CWizXmlRpcValue* pParam3, CWizXmlRpcValue* pParam4, \
                          CWizXmlRpcValue* pParam5, CWizXmlRpcValue* pParam6, \
                          CWizXmlRpcValue* pParam7, CWizXmlRpcValue* pParam8);


CWizXmlRpcRequest::CWizXmlRpcRequest(const QString& strMethodName)
{
    CWizXMLNode nodeMethodCall;
    m_doc.AppendChild("methodCall", nodeMethodCall);
    nodeMethodCall.SetChildNodeText("methodName", strMethodName);

    CWizXMLNode nodeParams;
    nodeMethodCall.AppendChild("params", nodeParams);
}

void CWizXmlRpcRequest::addParam(CWizXmlRpcValue* pParam)
{
    Q_ASSERT(pParam);

    CWizXMLNode nodeParams;
    m_doc.FindNodeByPath("methodCall/params", nodeParams);

    CWizXMLNode nodeParamValue;
    nodeParams.AppendNodeByPath("param/value", nodeParamValue);

    pParam->Write(nodeParamValue);
}

QByteArray CWizXmlRpcRequest::toData()
{
    QString strText;
    if (!m_doc.ToXML(strText, true)) {
        TOLOG("Failed to get xml text!");
        return QByteArray();
    }

    if (strText.length() < 10) {
        TOLOG1("Invalidate xml: %1", strText);
        return QByteArray();
    }

    if (strText[0] == '<' && strText[1] == '?')
    {
    }
    else
    {
        strText.insert(0, "<?xml version=\"1.0\"?>\n");
    }

    return strText.toUtf8();
}


CWizXmlRpcIntValue::CWizXmlRpcIntValue(int n /*= 0*/)
	: m_n(n)
{

}

bool CWizXmlRpcIntValue::Write(CWizXMLNode& nodeValue)
{
    nodeValue.SetChildNodeText("int", WizIntToStr(m_n));
    return true;
}

bool CWizXmlRpcIntValue::Read(CWizXMLNode& nodeValue)
{
    CString strValue = nodeValue.GetFirstChildNodeText();
    m_n = _ttoi(strValue);

    return true;
}

CString CWizXmlRpcIntValue::ToString() const
{
	return WizIntToStr(m_n);
}

CWizXmlRpcIntValue::operator int()
{
	return m_n;
}

CWizXmlRpcBoolValue::CWizXmlRpcBoolValue(BOOL b /*= FALSE*/)
	: m_b(b)
{

}

BOOL CWizXmlRpcBoolValue::Write(CWizXMLNode& nodeValue)
{
	nodeValue.SetChildNodeText(_T("boolean"), WizIntToStr(m_b ? 1 : 0));
	return TRUE;
}

BOOL CWizXmlRpcBoolValue::Read(CWizXMLNode& nodeValue)
{
	CString strValue = nodeValue.GetFirstChildNodeText();
    m_b = (strValue == _T("1") || 0 == strValue.CompareNoCase(_T("true")));

	return TRUE;
}

CString CWizXmlRpcBoolValue::ToString() const
{
	return WizIntToStr(m_b ? 1 : 0);
}

CWizXmlRpcBoolValue::operator BOOL ()
{
	return m_b;
}


CWizXmlRpcStringValue::CWizXmlRpcStringValue(const CString& strDef)
: m_str(strDef)
{

}

BOOL CWizXmlRpcStringValue::Write(CWizXMLNode& nodeValue)
{
	nodeValue.SetChildNodeText(_T("string"), m_str);
	return TRUE;
}

BOOL CWizXmlRpcStringValue::Read(CWizXMLNode& nodeValue)
{
	m_str = nodeValue.GetFirstChildNodeText();
	return TRUE;
}

CString CWizXmlRpcStringValue::ToString() const
{
	return m_str;
}

CWizXmlRpcStringValue::operator CString()
{
	return m_str;
}

CWizXmlRpcTimeValue::CWizXmlRpcTimeValue(const COleDateTime& t /*= ::WizGetCurrentTime()*/)
	: m_t(t)
{

}

BOOL CWizXmlRpcTimeValue::Write(CWizXMLNode& nodeValue)
{
    CString str = WizDateTimeToIso8601String(m_t);
    nodeValue.SetChildNodeText(_T("dateTime.iso8601"), str);
	return TRUE;
}

BOOL CWizXmlRpcTimeValue::Read(CWizXMLNode& nodeValue)
{
	CString str = nodeValue.GetFirstChildNodeText();
	//
	CString strError;
	if (!WizIso8601StringToDateTime(str, m_t, strError))
	{
		TOLOG(strError);
		return FALSE;
	}
	//
	return TRUE;
}

CString CWizXmlRpcTimeValue::ToString() const
{
	return ::WizDateTimeToString(m_t);
}

CWizXmlRpcTimeValue::operator COleDateTime()
{
	return m_t;
}


CWizXmlRpcBase64Value::CWizXmlRpcBase64Value(const QByteArray& arrayData)
    : m_arrayData(arrayData)
{

}

BOOL CWizXmlRpcBase64Value::Write(CWizXMLNode& nodeValue)
{
    CString strText;
    WizBase64Encode(m_arrayData, strText);
	nodeValue.SetChildNodeText(_T("base64"), CString(strText));
	return TRUE;
}

BOOL CWizXmlRpcBase64Value::Read(CWizXMLNode& nodeValue)
{
	CString str = nodeValue.GetFirstChildNodeText();
	//
    m_arrayData.clear();;
	//
    return WizBase64Decode(str, m_arrayData);
}

CString CWizXmlRpcBase64Value::ToString() const
{
    CString strText;
    WizBase64Encode(m_arrayData, strText);
	return CString(strText);
}

BOOL CWizXmlRpcBase64Value::GetStream(QByteArray& arrayData)
{
    arrayData = m_arrayData;
    return TRUE;
}


CWizXmlRpcArrayValue::CWizXmlRpcArrayValue()
{

}

CWizXmlRpcArrayValue::CWizXmlRpcArrayValue(const CWizStdStringArray& arrayData)
{
	SetStringArray(arrayData);
}

CWizXmlRpcArrayValue::~CWizXmlRpcArrayValue ()
{
	Clear();
}

BOOL CWizXmlRpcArrayValue::Write(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeData;
	nodeValue.AppendNodeByPath(_T("array/data"), nodeData);
	//
    for (std::deque<CWizXmlRpcValue*>::const_iterator it = m_array.begin();
		it != m_array.end();
		it++)
	{
		CWizXmlRpcValue* pValue = *it;
		//
		CWizXMLNode nodeElementValue;
		nodeData.AppendChild(_T("value"), nodeElementValue);
		//
		pValue->Write(nodeElementValue);
	}
	//
	return TRUE;
}

BOOL CWizXmlRpcArrayValue::Read(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeData;
	if (!nodeValue.FindNodeByPath(_T("array/data"), nodeData))
	{
		TOLOG(_T("Failed to get array data node!"));
		return FALSE;
	}
	//
    std::deque<CWizXMLNode> arrayValue;
	nodeData.GetAllChildNodes(arrayValue);
	//
    for (std::deque<CWizXMLNode>::iterator it = arrayValue.begin();
		it != arrayValue.end();
		it++)
	{
		CWizXMLNode& nodeElementValue = *it;
		//
		CWizXmlRpcValue* pElementValue = NULL;
		if (!WizXmlRpcValueFromXml(nodeElementValue, &pElementValue ))
		{
			TOLOG(_T("Failed to load array element value from node!"));
			return FALSE;
		}
		//
		ATLASSERT(pElementValue);
		//
		Add(pElementValue);
	}
	//
	return TRUE;
}

CString CWizXmlRpcArrayValue::ToString() const
{
	return CString(_T("[array]"));
}

void CWizXmlRpcArrayValue::Add(CWizXmlRpcValue* pValue)
{
	m_array.push_back(pValue);
}

void CWizXmlRpcArrayValue::Clear()
{
    for (std::deque<CWizXmlRpcValue*>::const_iterator it = m_array.begin();
		it != m_array.end();
		it++)
	{
		delete *it;
    }

	m_array.clear();
}

BOOL CWizXmlRpcArrayValue::ToStringArray(CWizStdStringArray& arrayRet)
{
    for (std::deque<CWizXmlRpcValue*>::const_iterator it = m_array.begin();
		it != m_array.end();
		it++)
	{
		CWizXmlRpcStringValue* pValue = dynamic_cast<CWizXmlRpcStringValue*>(*it);
		if (!pValue)
		{
			TOLOG(_T("Fault error: element of array is null or not a string"));
			return FALSE;
		}
		//
		arrayRet.push_back(pValue->ToString());
	}
	//
	return TRUE;
}

BOOL CWizXmlRpcArrayValue::SetStringArray(const CWizStdStringArray& arrayData)
{
	for (CWizStdStringArray::const_iterator it = arrayData.begin();
		it != arrayData.end();
		it++)
	{
		Add(new CWizXmlRpcStringValue(*it));
    }

	return TRUE;
}



void CWizXmlRpcStructValue::AddValue(const CString& strName, CWizXmlRpcValue* pValue)
{
    RemoveValue(strName);
	//
    m_map[CString(strName)] = pValue;
}

CWizXmlRpcValue* CWizXmlRpcStructValue::GetValue(const CString& strName) const
{
    std::map<CString, CWizXmlRpcValue*>::const_iterator  it = m_map.find(strName);
	if (it == m_map.end())
		return NULL;
	//
	return it->second;
}

CWizXmlRpcStructValue::~CWizXmlRpcStructValue()
{
	Clear();
}

BOOL CWizXmlRpcStructValue::Write(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeStruct;
	nodeValue.AppendChild(_T("struct"), nodeStruct);
	//
	for (std::map<CString, CWizXmlRpcValue*>::const_iterator it = m_map.begin();
		it != m_map.end();
		it++)
	{
		CString strName = it->first;
		CWizXmlRpcValue* pValue = it->second;
		//
		CWizXMLNode nodeMember;
		nodeStruct.AppendChild(_T("member"), nodeMember);
		//
		nodeMember.SetChildNodeText(_T("name"), strName);
		//
		CWizXMLNode nodeElementValue;
		nodeMember.AppendChild(_T("value"), nodeElementValue);
		//
		pValue->Write(nodeElementValue);
	}
	//
	return TRUE;
}

bool CWizXmlRpcStructValue::Read(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeStruct;
	if (!nodeValue.FindChildNode(_T("struct"), nodeStruct))
	{
		TOLOG(_T("Failed to get struct node!"));
		return FALSE;
	}
	//
    std::deque<CWizXMLNode> arrayMember;
	nodeStruct.GetAllChildNodes(arrayMember);
	//
    for (std::deque<CWizXMLNode>::iterator it = arrayMember.begin();
		it != arrayMember.end();
		it++)
	{
		CWizXMLNode& nodeMember = *it;
		//
		CString strName;
		if (!nodeMember.GetChildNodeText(_T("name"), strName))
		{
			TOLOG(_T("Failed to get struct member name!"));
			return FALSE;
		}
		//
		CWizXMLNode nodeMemberValue;
		if (!nodeMember.FindChildNode(_T("value"), nodeMemberValue))
		{
			TOLOG(_T("Failed to get struct member value!"));
			return FALSE;
		}
		//
		CWizXmlRpcValue* pMemberValue = NULL;
        if (!WizXmlRpcValueFromXml(nodeMemberValue, &pMemberValue))
		{
			TOLOG(_T("Failed to load struct member value from node!"));
			return FALSE;
		}
		//
		ATLASSERT(pMemberValue);
		//
		AddValue(strName, pMemberValue);
	}
	//
	return TRUE;
}

CString CWizXmlRpcStructValue::ToString() const
{
	return CString(_T("[struct]"));
}

void CWizXmlRpcStructValue::AddInt(const CString& strName, int n)
{
    AddValue(strName, new CWizXmlRpcIntValue(n));
}

void CWizXmlRpcStructValue::AddString(const CString& strName, const CString& str)
{
    AddValue(strName, new CWizXmlRpcStringValue(str));
}

void CWizXmlRpcStructValue::AddBool(const CString& strName, BOOL b)
{
    AddValue(strName, new CWizXmlRpcBoolValue(b));
}

void CWizXmlRpcStructValue::AddTime(const CString& strName, const COleDateTime& t)
{
    AddValue(strName, new CWizXmlRpcTimeValue(t));
}

void CWizXmlRpcStructValue::AddBase64(const CString& strName, const QByteArray& arrayData)
{
    AddValue(strName, new CWizXmlRpcBase64Value(arrayData));
}

void CWizXmlRpcStructValue::AddStringArray(const CString& strName, const CWizStdStringArray& arrayData)
{
    AddValue(strName, new CWizXmlRpcArrayValue(arrayData));
}

void CWizXmlRpcStructValue::AddInt64(const CString& strName, __int64 n)
{
    AddValue(strName, new CWizXmlRpcStringValue(WizInt64ToStr(n)));
}

void CWizXmlRpcStructValue::AddColor(const CString& strName, COLORREF cr)
{
    AddValue(strName, new CWizXmlRpcStringValue(WizColorToString(cr)));
}

BOOL CWizXmlRpcStructValue::AddFile(const CString& strName, const CString& strFileName)
{
    Q_UNUSED(strName);
    Q_UNUSED(strFileName);
    ATLASSERT(FALSE);
	return TRUE;
}

void CWizXmlRpcStructValue::AddStruct(const CString& strName, CWizXmlRpcStructValue* pStruct)
{
    AddValue(strName, pStruct);
}

void CWizXmlRpcStructValue::AddArray(const CString& strName, CWizXmlRpcValue* pArray)
{
    AddValue(strName, pArray);
}

void CWizXmlRpcStructValue::Clear()
{
	for (std::map<CString, CWizXmlRpcValue*>::const_iterator it = m_map.begin();
		it != m_map.end();
		it++)
	{
		DeleteValue(it->second);
	}
	//
	m_map.clear();
}

void CWizXmlRpcStructValue::RemoveValue(const CString& strName)
{
    std::map<CString, CWizXmlRpcValue*>::iterator  it = m_map.find(strName);
	if (it == m_map.end())
		return;
	//
	DeleteValue(it->second);
	//
	m_map.erase(it);
}

void CWizXmlRpcStructValue::DeleteValue(CWizXmlRpcValue* pValue)
{
	delete pValue;
}

BOOL CWizXmlRpcStructValue::GetBool(const CString& strName, BOOL& b) const
{
    CWizXmlRpcBoolValue* p = GetValue<CWizXmlRpcBoolValue>(strName);
	if (!p)
		return FALSE;
	//
	b = *p;
	//
	return TRUE;
	
}

BOOL CWizXmlRpcStructValue::GetInt(const CString& strName, int& n) const
{
    if (CWizXmlRpcIntValue* p = GetValue<CWizXmlRpcIntValue>(strName))
	{
		n = *p;
		return TRUE;
	}
    if (CWizXmlRpcStringValue* p = GetValue<CWizXmlRpcStringValue>(strName))
	{
		n = _ttoi(CString(*p));
		return TRUE;
    }

	return FALSE;
}

bool CWizXmlRpcStructValue::GetString(const QString& strName, QString& str) const
{
    if (CWizXmlRpcIntValue* p = GetValue<CWizXmlRpcIntValue>(strName))
	{
		str = WizIntToStr(*p);
		return TRUE;
	}
    if (CWizXmlRpcStringValue* p = GetValue<CWizXmlRpcStringValue>(strName))
	{
		str = *p;
		return TRUE;
    }

	return FALSE;
}

BOOL CWizXmlRpcStructValue::GetTime(const CString& strName, COleDateTime& t) const
{
    if (CWizXmlRpcTimeValue* p = GetValue<CWizXmlRpcTimeValue>(strName))
	{
		t = *p;
		return TRUE;
    }

	return FALSE;
}

BOOL CWizXmlRpcStructValue::GetStream(const CString& strName, QByteArray& arrayData) const
{
    if (CWizXmlRpcBase64Value* p = GetValue<CWizXmlRpcBase64Value>(strName))
    {
        return p->GetStream(arrayData);
    }

    return FALSE;
}

CWizXmlRpcStructValue* CWizXmlRpcStructValue::GetStruct(const CString& strName) const
{
    return GetValue<CWizXmlRpcStructValue>(strName);
}

CWizXmlRpcArrayValue* CWizXmlRpcStructValue::GetArray(const CString& strName) const
{
    return GetValue<CWizXmlRpcArrayValue>(strName);
}

BOOL CWizXmlRpcStructValue::GetInt64(const CString& strName, __int64& n) const
{
	CString str;
    if (!GetStr(strName, str))
		return FALSE;
	//
	n = _ttoi64(str);
	return TRUE;
}

BOOL CWizXmlRpcStructValue::GetInt(const CString& strName, long& n) const
{
	int i = 0;
    if (!GetInt(strName, i))
		return FALSE;
	//
	n = i;
	return TRUE;
}

BOOL CWizXmlRpcStructValue::GetColor(const CString& strName, COLORREF& cr) const
{
	CString str;
    if (!GetStr(strName, str))
	{
        TOLOG1(_T("Failed to get member %1"), strName);
		return FALSE;
	}
	cr = WizStringToColor(str);
	//
	return TRUE;
}

BOOL CWizXmlRpcStructValue::GetStringArray(const CString& strName, CWizStdStringArray& arrayData) const
{
    CWizXmlRpcArrayValue* pArray = GetArray(strName);
	if (!pArray)
	{
		TOLOG(_T("Failed to get array data in struct"));
		return FALSE;
	}
	//
	return pArray->ToStringArray(arrayData);
}

BOOL CWizXmlRpcStructValue::ToStringMap(std::map<CString, CString>& ret) const
{
	for (std::map<CString, CWizXmlRpcValue*>::const_iterator it = m_map.begin();
		it != m_map.end();
		it++)
	{
		ret[it->first] = it->second->ToString();
	}
	//
	return TRUE;
}


CWizXmlRpcFaultValue::~CWizXmlRpcFaultValue()
{

}

bool CWizXmlRpcFaultValue::Read(CWizXMLNode& nodeValue)
{
	return m_val.Read(nodeValue);
}

int CWizXmlRpcFaultValue::GetFaultCode() const
{
	int nCode = 0;
	m_val.GetInt(_T("faultCode"), nCode);
	return nCode;
}

CString CWizXmlRpcFaultValue::GetFaultString() const
{
	CString str;
	m_val.GetString(_T("faultString"), str);
	return str;
}


BOOL WizXmlRpcParamsToXml(CWizXMLDocument& doc, const CString& strMethodName, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2, CWizXmlRpcValue* pParam3, CWizXmlRpcValue* pParam4,
						  CWizXmlRpcValue* pParam5, CWizXmlRpcValue* pParam6, CWizXmlRpcValue* pParam7, CWizXmlRpcValue* pParam8)
{
	CWizXMLNode nodeMethodCall;
	doc.AppendChild(_T("methodCall"), nodeMethodCall);
	//
    nodeMethodCall.SetChildNodeText(_T("methodName"), strMethodName);
	//

	CWizXMLNode nodeParams;
	nodeMethodCall.AppendChild(_T("params"), nodeParams);
	//
	CWizXmlRpcValue* arrayParams[] = {pParam1, pParam2, pParam3, pParam4, pParam5, pParam6, pParam7, pParam8, NULL};
	//
	CWizXmlRpcValue** ppParam = arrayParams;

	while (*ppParam)
	{
		CWizXMLNode nodeParamValue;
		nodeParams.AppendNodeByPath(_T("param/value"), nodeParamValue);
		//
		CWizXmlRpcValue* pParam = *ppParam;
		//
		pParam->Write(nodeParamValue);
		//
		ppParam++;
	}
	//
	return TRUE;
}

BOOL WizXmlRpcValueFromXml(CWizXMLNode& nodeValue, CWizXmlRpcValue** ppRet)
{
	*ppRet = NULL;
	//
	CWizXMLNode nodeTypeTest;
	nodeValue.GetFirstChildNode(nodeTypeTest);
    if (nodeTypeTest.isNull())
	{
		CString strText = nodeValue.GetText();
		//
		*ppRet = new CWizXmlRpcStringValue(strText);
		return TRUE;
	}
	//
	CWizXmlRpcValue* pValue = NULL;
	//
	CString strValueType = nodeTypeTest.GetName();
	//
	if (0 == strValueType.CompareNoCase(_T("int"))
		|| 0 == strValueType.CompareNoCase(_T("i4")))
	{
		pValue = new CWizXmlRpcIntValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("string"))
		|| 0 == strValueType.CompareNoCase(_T("ex:nil"))
		|| 0 == strValueType.CompareNoCase(_T("ex:i8")))
	{
		pValue = new CWizXmlRpcStringValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("nil"))
		|| 0 == strValueType.CompareNoCase(_T("i8")))
	{
		pValue = new CWizXmlRpcStringValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("struct")))
	{
		pValue = new CWizXmlRpcStructValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("array")))
	{
		pValue = new CWizXmlRpcArrayValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("base64")))
	{
		pValue = new CWizXmlRpcBase64Value();
	}
	else if (0 == strValueType.CompareNoCase(_T("boolean"))
		|| 0 == strValueType.CompareNoCase(_T("bool")))
	{
		pValue = new CWizXmlRpcBoolValue();
	}
	else if (0 == strValueType.CompareNoCase(_T("dateTime.iso8601")))
	{
		pValue = new CWizXmlRpcTimeValue();
	}
	else
	{
		TOLOG1(_T("Unknown xmlrpc value type:%1"), strValueType);
		return FALSE;
	}
	//

	//
	ATLASSERT(pValue);
	//
	if (pValue->Read(nodeValue))
	{
		*ppRet = pValue;
		return TRUE;
	}
	//
	TOLOG(_T("Failed to read xmlrpc value!"));
	//
	delete pValue;
	//
	return FALSE;
}


bool WizXmlRpcResultFromXml(CWizXMLDocument& doc, CWizXmlRpcValue** ppRet)
{
	CWizXMLNode nodeMethodResponse;
    doc.FindChildNode("methodResponse", nodeMethodResponse);
    if (nodeMethodResponse.isNull())
	{
        TOLOG("Failed to get methodResponse node!");
        return false;
	}

	CWizXMLNode nodeTest;
	if (!nodeMethodResponse.GetFirstChildNode(nodeTest))
	{
        TOLOG("Failed to get methodResponse child node!");
        return false;
	}

	CString strTestName = nodeTest.GetName();

    if (0 == strTestName.CompareNoCase("params"))
	{
		CWizXMLNode nodeParamValue;
        nodeTest.FindNodeByPath("param/value", nodeParamValue);
        if (nodeParamValue.isNull())
		{
            TOLOG("Failed to get param value node of params!");
            return false;
		}

		return WizXmlRpcValueFromXml(nodeParamValue, ppRet);
	}
    else if (0 == strTestName.CompareNoCase("fault"))
	{
		CWizXMLNode nodeFaultValue;
		nodeTest.FindChildNode(_T("value"), nodeFaultValue);
        if (nodeFaultValue.isNull())
		{
            TOLOG("Failed to get fault value node!");
            return false;
		}

		CWizXmlRpcFaultValue* pFault = new CWizXmlRpcFaultValue();
		pFault->Read(nodeFaultValue);

		*ppRet = pFault;

        return true;
	}
	else
	{
        TOLOG1("Unknown response node name: %1", strTestName);
        return false;
	}
}
