class CCameraViewfinder : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CCameraViewfinder, vgui::Panel);

public:
	CCameraViewfinder(const char* pElementName);

	void Init();
	void MsgFunc_ShowCameraViewfinder(bf_read& msg);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual void Paint(void);

private:
	bool			m_bShow;
	CHudTexture* m_pViewfinder_ul;
	CHudTexture* m_pViewfinder_halfcircle;
};