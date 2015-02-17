package org.urobot.media;

import java.util.ArrayList;

import org.teleal.cling.support.model.container.Container;
import org.teleal.cling.support.model.item.Item;


public class ContentList {
	public static final int IMAGEITEM=0;
	public static final int AUDIOITEM=1;
	public static final int VIDEOITEM=2;

	private ArrayList<String> titlelist;
	private ArrayList<String> urilist;
	private int itemType;
	int childItems = 0;

	/**
	 * @param container
	 * @param item
	 */
	public ContentList(int type) {
		super();
		titlelist = new ArrayList<String>();
		urilist = new ArrayList<String>();
		itemType = type;
	}
	
	public ArrayList<String> getTitleList() {
		return titlelist;
	}
	
	public ArrayList<String> getUriList() {
		return urilist;
	}
	
	public void addContent(ContentNode childNode) {
		if (!childNode.isContainer()) {
			String className = childNode.getItem().getClazz().getValue();
			if (itemType == VIDEOITEM && className.contains("videoItem")) {
				titlelist.add(childNode.getItem().getTitle());
				urilist.add(childNode.getItem().getFirstResource().getValue());
				this.childItems++;
			}
			else if (itemType == AUDIOITEM && className.contains("audioItem")) {
				titlelist.add(childNode.getItem().getTitle());
				urilist.add(childNode.getItem().getFirstResource().getValue());
				this.childItems++;
			}
			else if (itemType == IMAGEITEM && className.contains("imageItem")) {
				titlelist.add(childNode.getItem().getTitle());
				urilist.add(childNode.getItem().getFirstResource().getValue());
				this.childItems++;
			}
		}
	}
	
	public void addContentList(ArrayList<ContentNode> childNodes) {
		for (ContentNode childNode : childNodes)
			addContent(childNode);
	}
	
	public int getItemIndex(ContentNode node) {
		int index = 0;
		for (String title : titlelist) {
			String titleName = node.getItem().getTitle();
			if (title.equals(titleName))
				return index;
			index++;
		}
		return -1;

	}
	
}