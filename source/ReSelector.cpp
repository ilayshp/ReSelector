/**************************************************************************************
**
**  Copyright (C) 2007-2009 Remotion. All rights reserved.
**	
**  This file is part of the ReSelector from www.remotion4d.net.
**
**	The MIT license:
**
**	Permission is hereby granted, free of charge, to any person obtaining a copy 
**	of this software and associated documentation files (the "Software"), to deal 
**	in the Software without restriction, including without limitation the rights 
**	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
**	copies of the Software, and to permit persons to whom the Software is 
**	furnished to do so, subject to the following conditions:
**
**	The above copyright notice and this permission notice shall be included 
**	in all copies or substantial portions of the Software.
**
**	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
**	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
**	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
**	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
**	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
**	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
***************************************************************************************/

// ReSelector Unique ID
#define ID_RESELECTOR 1012220

#include "c4d.h"
#include "c4d_symbols.h"


//==============================================================================
//############################### ReSelector ##################################
//==============================================================================
class ReSelector : public GeDialog
{
private:

public:
	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	//virtual Int32 Message(const BaseContainer &msg,BaseContainer &result);
	virtual Bool CoreMessage  (Int32 id, const BaseContainer& msg);
};
//-----------------------------------------------------------------------------
enum 
{
	IDC_SEED		= 1001,
	IDC_PROB		= 1002,
	IDC_TYPE		= 1003,
	IDC_MODUS		= 1004,
	IDC_ASEED		= 1005,
	IDC_SELECT		= 1010,
	IDC_RESEED		= 1020,
	IDC_INFO		= 1030,
	IDC_DUMMY_
};
//-----------------------------------------------------------------------------
Bool ReSelector::CreateLayout()
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	SetTitle(GeLoadString(IDS_RE_SELECTOR));

	GroupBegin(0, BFH_SCALEFIT, 1, 0, String(), 0);
	{
		GroupBorderSpace(4, 4, 4, 4);
		GroupBorderNoTitle(BORDER_THIN_OUT);

		GroupBegin(0, BFH_SCALEFIT, 2, 0, String(), 0);
		{	
			AddStaticText(0, BFH_LEFT, 0, 0, "Type"_s, 0);
			AddComboBox(IDC_TYPE, BFH_SCALEFIT);
				AddChild(IDC_TYPE, 0, "Point"_s);
				AddChild(IDC_TYPE, 1, "Edges"_s);
				AddChild(IDC_TYPE, 2, "Polygons"_s);

			AddStaticText(0, BFH_LEFT, 0, 0, "Modus"_s, 0);
			AddComboBox(IDC_MODUS, BFH_SCALEFIT);
				AddChild(IDC_MODUS, 0, "Replace"_s);
				AddChild(IDC_MODUS, 1, "Add"_s);
				AddChild(IDC_MODUS, 2, "Sub"_s);
		}
		GroupEnd();

		AddSeparatorH(0);

		GroupBegin(0, BFH_SCALEFIT, 2, 0, String(), 0);
		{	
			AddStaticText(0, BFH_LEFT, 0, 0, "Probability"_s, 0);
			AddEditSlider(IDC_PROB, BFH_SCALEFIT);
		}
		GroupEnd();

		AddSeparatorH(0);

		GroupBegin(0, BFH_SCALEFIT, 3, 0, String(), 0);
		{	
			
			AddCheckbox(IDC_ASEED, BFH_LEFT, 0, 0, "Seed"_s);
			//AddStaticText(0,BFH_LEFT,0,0,"Seed",0);
			AddEditNumberArrows(IDC_SEED, BFH_LEFT);
			AddButton(IDC_RESEED, BFH_FIT, 0, 0, "Reseed"_s);
		}
		GroupEnd();

		AddSeparatorH(0);
		AddButton(IDC_SELECT, BFH_FIT, 0, 0, "Select"_s);
	}
	GroupEnd();

	MenuFlushAll();	
		MenuSubBegin("About"_s);
			MenuAddString(IDC_INFO, "Remotion 2007"_s);
			MenuAddString(IDC_INFO, "ReSelector 0.2"_s);
			MenuAddString(IDC_INFO, "www.remotion4d.net"_s);
		MenuSubEnd();
	MenuFinished();
	return res;
}
//-----------------------------------------------------------------------------
Bool ReSelector::InitValues()
{
	// first call the parent instance
	if (!GeDialog::InitValues()) 
		return FALSE;

	SetInt32(IDC_SEED, 0l, 0, 999999, 1, 0);
	SetPercent(IDC_PROB, 0.5, 0.0);

	// Enable
	Bool ased; 
	GetBool(IDC_ASEED, ased);
	Enable(IDC_SEED, ased);
	Enable(IDC_RESEED, ased);

	return TRUE;
}
//-----------------------------------------------------------------------------
Bool ReSelector::CoreMessage(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case EVMSG_TOOLCHANGED:
			if (CheckCoreMessage(msg))
			{
				BaseDocument	*doc = GetActiveDocument(); 
				if (!doc) 
					return FALSE;
				Int32 mode = doc->GetMode();
				
				switch (mode)
				{
					case Mpoints: SetInt32(IDC_TYPE, 0); break;
					case Medges: SetInt32(IDC_TYPE, 1); break;
					case Mpolygons: SetInt32(IDC_TYPE, 2); break;
				}
			}
			break;
	}
	return GeDialog::CoreMessage(id, msg);
}
//-----------------------------------------------------------------------------
/// This is simple algorithm of ReSelector >>>>>>>>>>>
Bool ReSelector::Command(Int32 id, const BaseContainer& msg)
{
	if (id == IDC_SELECT)
	{
		Int32 seed; GetInt32(IDC_SEED, seed);
		Int32 mode; GetInt32(IDC_TYPE, mode);
		Int32 art;  GetInt32(IDC_MODUS, art);
		Float prob; GetFloat(IDC_PROB, prob);
		Bool ased; GetBool(IDC_ASEED, ased);

		BaseDocument	*doc = GetActiveDocument();
		if (!doc)
			return FALSE;
		PolygonObject	*ob = (PolygonObject*)doc->GetActiveObject();
		if (!ob)
			return FALSE;
		Random			rnd;
		if (ased)
			rnd.Init(seed);
		else
			rnd.Init(GeGetTimer());


		switch (mode)
		{
			// point
			case 0:
			{
				if (!ob->IsInstanceOf(Opoint))
					return FALSE;

				BaseSelect *psel = ob->GetPointS();
				if (!psel)
					return FALSE;

				const Int32 pcnt = ob->GetPointCount();
				// Replace
				if (art == 0)
				{
					for (Int32 c = 0; c < pcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
						else
							psel->Deselect(c);
					}
				}
				else if (art == 1)
				{
					// Add
					for (Int32 c = 0; c < pcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
					}

				}
				else if (art == 2)
				{
					// Sub
					for (Int32 c = 0; c < pcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Deselect(c);
					}
				}
				ob->Message(MSG_UPDATE);
				EventAdd();
				break;
			}

			// selection of edge
			case 1:
			{
				if (!ob->IsInstanceOf(Opolygon))
					return FALSE;

				// BaseSelect *psel = ob->GetEdgeS(); 
				// if(!psel) 
				//	return FALSE;

				const Int32 pcnt = ob->GetPointCount();
				// const Vector* padr = ob->GetPointR();

				const Int32 vcnt = ob->GetPolygonCount();
				const CPolygon* vadr = ob->GetPolygonR();

				Neighbor ng;
				ng.Init(pcnt, vadr, vcnt, NULL);

				Int32 ecnt = ng.GetEdgeCount();
				BaseSelect *psel = ob->GetSelectedEdges(&ng, EDGESELECTIONTYPE::SELECTION);
				if (!psel)
					return FALSE;

				// Replace
				if (art == 0)
				{
					for (Int32 c = 0; c < ecnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
						else
							psel->Deselect(c);
					}
				}
				// add
				else if (art == 1)
				{
					for (Int32 c = 0; c < ecnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
						// else if(psel->IsSelected(c)) psel->Select(c);
					}
				}
				// sub
				else if (art == 2)
				{
					for (Int32 c = 0; c < ecnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Deselect(c);
						// else if(psel->IsSelected(c)) psel->Select(c);
					}
				}
				ob->SetSelectedEdges(&ng, psel, EDGESELECTIONTYPE::SELECTION);
				BaseSelect::Free(psel);
				ob->Message(MSG_UPDATE);
				EventAdd();
				break;
			}
			// polygons
			case 2:
			{
				if (!ob->IsInstanceOf(Opolygon))
					return FALSE;
				BaseSelect *psel = ob->GetPolygonS();
				if (!psel)
					return FALSE;

				const Int32 vcnt = ob->GetPolygonCount();
				// Replace
				if (art == 0)
				{
					for (Int32 c = 0; c < vcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
						else
							psel->Deselect(c);
					}

				}
				// Add
				else if (art == 1)
				{
					for (Int32 c = 0; c < vcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Select(c);
					}
				}
				// Sub
				else if (art == 2)
				{
					for (Int32 c = 0; c < vcnt; ++c)
					{
						if (rnd.Get01() < prob)
							psel->Deselect(c);
					}
				}
				ob->Message(MSG_UPDATE);
				EventAdd();
				break;
			}
			
		}
	}
	else if (id == IDC_RESEED)
	{
		SetInt32(IDC_SEED, Int32((GeGetTimer() * 75) % 999999), 0, 999999);
	}
	else if (id == IDC_ASEED)
	{
		// Enable
		Bool ased; GetBool(IDC_ASEED, ased);
		Enable(IDC_SEED, ased);
		Enable(IDC_RESEED, ased);
	}
	else if (id == IDC_INFO)
	{
		GeOpenHTML("http://www.remotion4d.net"_s);
	}
	return TRUE;
} 

//==============================================================================
//############################# ReSelectorDlg #################################
//==============================================================================
class ReSelectorDlg : public CommandData
{
	private:
		ReSelector dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual Int32 GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};
//-----------------------------------------------------------------------------
Int32 ReSelectorDlg::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}
//-----------------------------------------------------------------------------
Bool ReSelectorDlg::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE::ASYNC, ID_RESELECTOR, -1, -1);
}
//-----------------------------------------------------------------------------
Bool ReSelectorDlg::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(ID_RESELECTOR, 0, secret);
}

//#############################################################################
Bool RegisterReSelector()
{
	// decide by name if the plugin shall be registered - just for user convenience
	//String name=GeLoadString(IDS_RE_SELECTOR); if (!name.Content()) return TRUE;
	//return RegisterCommandPlugin(ID_RESELECTOR,name,0,"ReSelector.tif",name,gNew ReSelectorDlg);
	return RegisterCommandPlugin(ID_RESELECTOR, String("ReSelector"), 0, nullptr, String(), NewObjClear(ReSelectorDlg));
}

// Developed by Remotion :)
// www.remotion4d.net
