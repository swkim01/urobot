package org.urobot.media;

import java.util.ArrayList;

import org.teleal.cling.support.model.container.Container;
import org.teleal.cling.support.model.item.Item;


public class ContentNode {
	private boolean isStorageFolder;
	
	Container container;
	Item item;
	ContentNode parentNode;
	ArrayList<ContentNode> childNodes;
	int childContainers = 0;
	int childItems = 0;

	/**
	 * @param container
	 * @param item
	 */
	public ContentNode(ContentNode parent, Container container) {
		super();
		this.parentNode = parent;
		this.container = container;
		this.isStorageFolder = true;
	}
	
	public ContentNode(ContentNode parent, Item item) {
		super();
		this.parentNode = parent;
		this.item = item;
		this.isStorageFolder = false;
	}

	public boolean isContainer() {
		return isStorageFolder;
	}
	
	public Container getContainer() {
		return container;
	}
	
	public Item getItem() {
		return item;
	}
	
	public void setChildNodes(ArrayList<ContentNode> childNodes) {
		this.childNodes = childNodes;
	}
	
	public void addChildNode(ContentNode childNode) {
		if (this.childNodes == null)
			this.childNodes = new ArrayList<ContentNode>();
		this.childNodes.add(childNode);
		if (childNode.isContainer())
			this.childContainers++;
		else
			this.childItems++;
	}
	
	public ArrayList<ContentNode> getChildNodes() {
		return this.childNodes;
	}
	
	public int getChildContainers() {
		return this.childContainers;
	}
	
	public int getChildItems() {
		return this.childItems;
	}
	public ContentNode getParentNode() {
		return parentNode;
	}

}