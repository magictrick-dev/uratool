#ifndef URATOOL_GUI_VIEW_H
#define URATOOL_GUI_VIEW_H
#include <ncurses.h>

#define SPLIT_NONE 0
#define SPLIT_HORIZONTAL 1
#define SPIT_VERTICAL 2
typedef int split_t;

// -----------------------------------------------------------------------------
// I'm a little proud of this one:
// 		So, the problem with ncurses windows is that we aren't allowed to overlap
// 		them. In an effort to prevent that, we want to emulate vim's split feature
// 		by sub-dividing the screen into various regions. This methodology allows
// 		for dynamic windowing.
//
// 		What ends up happening is that when the main view splits, you get two
// 		branches from the main view, call them A and B. The view "A" is the
// 		main view, but smaller due to the split. The new view, "B" is the newly
// 		created view. If we perform another split on B, you get views C and D.
// 		C is actually just a partition B, and D is a new view. The pattern here
// 		is essentially a binary tree.
//
// 									A0
// 								 /      \
// 								A1      B0
// 						       /  \    /  \
// 						      A2  D0  B1  C0
//
//   	When we're rendering, we are rendering using a binary-tree traversal
//   	technique. If we are a leaf node, (SPLIT_NONE), then we "show" that
//   	window using provided dimensions of the window.
//
//   	The root node, A0, would therefore have the full X/Y dimensions of the
//   	terminal window. Lets call it width 400, height 300.
//
//   	Since A0 is not a leaf node, we would need to traverse lower. We look
//   	at nodes A1 and B0. What is interesting with this is that we still want
//   	to render view "A", just with smaller dimensions. However, B0 is an entirely
//   	different view separate from A, and therefore we want to render that one.
//
//   	If we perform a pre-order traversal where we cycle down from A0->AN, then
//   	render its adjacent children, you get a render order of:
//
//   	A2, D0, B1, C0.
//
//   	In order to properly calculate these dimensions, we would need some way
//   	of accessing the parent's properties to correctly size our views...
//
//   	During our pre-order traversal:
//   	1. 	Determine if the current node is a leaf, and if it is, render proper.
//   	
//   	2. 	If it isn't, then we calculate our split sizing. There are two types
//   		of split sizing, fixed or ratio.
//
//   		A.	If fixed, we take the view that is set fixed, subtract from
//   			dimension it fixes to determine the size of the adjacent view.
// 			B.  If ratio'd (lol) then determine which one is the receiving the
// 				portion in regards to the ratio (lol) and size.
//
// 		3. 	Once sizing is determine, recursively render the two views by going
// 			back to step 1.
//
// 		So how do we represent this behavior? Well, we would need to create a
// 		binary tree which holds the views and the splits. Since views are not
// 		tied to their split factor (views hold the responsibility of just rendering
// 		and maintaining the functionality of the view), a seperate data structure
// 		will be required to do this.
//
// 		RenderTree:
// 			rootnode:
// 				split_factor
// 				
// 				view_a = nullptr
// 				view_b = nullptr
//
// 			rootnode_withchildre:
// 				split_factor -> horizontal, ration, a:50%
// 				view_source = 
//
//
// -----------------------------------------------------------------------------

/**
 * Views are generally partitions of the main ncurses stdscr. In order to
 * accurately display various partitions, views recursively subdivide themselves.
 * The problem with ncurses' window schema is that they aren't allowed to overlap.
 * Therefore, to prevent this behavior, views which contain children do not self-invoke
 *
 */
class View
{
	public:

		inline void 	render();

	protected:
		View* 			_parent;
		View* 			_child_a;
		View* 			_child_b;

		split_t 		_split_type;
};

#if 1

View::
Render()
{

	// If the view isn't split, we can view it.
	if (_split_type == SPLIT_NONE)
	{
		this->view(this->x, this->y);
		return;
	}

	// However, if we are split, then we need to go deeper.
	if (this->_child_a == this)
	{
		// calculate the sub-division, assume 50/50
		this->view(this->x/2, this->y);
	}
	else
	{
		this->_child_a->rende
	}

}

#endif

#endif
