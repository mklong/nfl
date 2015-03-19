/** 
 * @file	cq_base.h
 * @brief	
 *  常用list结构，自带缓存对象功能，在对内存要求较高的场景下
 *  会尽量避免小块内存的频繁分配与释放，产生内存碎片。
 * 
 * detail...
 * 
 * @author	mklong
 * @version	1.0
 * @date	2014/6/5
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>2014/6/5	<td>mklong	<td>Create this file</tr>
 * </table>
 * 
 */
#ifndef __CQ_BASE_H__
#define __CQ_BASE_H__

#include "nfl_base.h"
#include "ace/Global_Macros.h"

namespace nfl{


/** 
 * @class	cq_item_base
 * 
 * @brief	
 *		list结构的对象基类，cleanup是对象的释放方法，
 *   通常设置了cleanup_fn后，是调用list中的方法释放，
 *   释放的方式可以是放回缓存，单独的栈对象无需释放
 *
 * detail...
 * 
 * @author	mklong
 * @date	2014/6/5
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
	struct cq_item_base
	{
		//f
		typedef void (* cleanup_fn)(cq_item_base * item,void* ptr);

	public:

		cq_item_base()
			:next_(NULL),
			fn_(NULL),
			ptr_(NULL){}


		void cleanup(){
			if (fn_)
				(*fn_)(this,ptr_);
		}

		void set_cleanup(cleanup_fn fn,void * ptr){
			fn_ = fn;
			ptr_ = ptr;
		}

		size_t id()const
		{
			return id_;
		}

		void id(size_t n){
			id_ = n;
		}

	public:
		cq_item_base *next_;

	private:
		size_t id_;
		cleanup_fn fn_;
		void*	ptr_;
	};


	/** 
 * @class	cq_base
 * 
 * @brief	
 *		list类，附带对象缓存和回收功能，并提供线程安全模板
 *
 * detail...
 *
 *		模板中第二个参数是用来指定锁的类型，
 *      一般该数据结构如果跨线程使用，并需要回收缓存
 *		对象，就得在模板中传入ACE_SYNCH,该宏在有线程的系统下
 *		会实现mutex，如果传入ACE_NULL_SYNCH,锁都是空实现，
 *		这种情况下需要使用者自己保证线程安全
 *		需要注意的是从哪个base调用cqi_new得到的对象，尽量调用该
 *		对象的cleanup将其归还，或者调用该base的cq_free，如果调用另外
 *		一个base的cq_free，缓存对象的功能就无效了。
 * 
 * @author	mklong
 * @date	2014/6/5
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
	template<class CQ_ITEM = cq_item_base,class _ACE_SYNCH = ACE_SYNCH>
	class cq_base : public nfl_nocopy
	{
	public:

		enum{
			MIN_PER_ALLOC = 3,
			MIN_PAGE_ALLOC = 100
		};

		cq_base():cur_id_(0),
			  total_alloc_(0),
			  per_alloc_(0),
			  cur_size_(0),
			  pagesize_(nfl_os::DEFAULT_PAGESIZE),
			  free_head_(NULL),
		      free_tail_(NULL),
			  head_(NULL),
			  tail_(NULL),
			  page_list_(NULL)
		{
			per_alloc_ = MIN_PAGE_ALLOC*pagesize_/sizeof(CQ_ITEM);
			if (per_alloc_ <= MIN_PER_ALLOC){
				per_alloc_ = MIN_PER_ALLOC;
			}
			
		}

		int open(size_t pagesize = nfl_os::DEFAULT_PAGESIZE)
		{
			pagesize_ = pagesize > nfl_os::PageSize ? pagesize:nfl_os::PageSize;
			per_alloc_ = MIN_PAGE_ALLOC*pagesize_/sizeof(CQ_ITEM);
			
			if (per_alloc_ <= MIN_PER_ALLOC){
				per_alloc_ = MIN_PER_ALLOC;
			}
			return 0;
		}
		
		int close()
		{
			CQ_ITEM *item;
			ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, ace_mon, this->mutex_, NULL);
			cur_id_ = 0;
			total_alloc_ = 0;

			cur_size_ = 0;

			free_head_ = NULL;
			free_tail_= NULL;
			head_ = NULL;
			tail_ = NULL;

			while(page_list_ != NULL){
				item = page_list_;
				page_list_ = (CQ_ITEM*)page_list_->next_;
				delete [] item;
			}
			
			return 0;
		}

		CQ_ITEM *	cqi_new()
		{
			CQ_ITEM *item = NULL;

			ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, ace_mon, this->mutex_, NULL);
			if (NULL != free_head_){
				item =(CQ_ITEM *)free_head_;
				free_head_ = item->next_;
				if (NULL == free_head_){
					free_tail_ = NULL;
				}	
			}

			//alloc free list
			if (NULL == item){
				int i;
				//optimize ,alloc pagesize 
				item = new CQ_ITEM[per_alloc_];
				if (NULL == item){
					return NULL;
				}

				//memory free list
				item[0].next_ = page_list_;
				page_list_ = item;

				//MIN_PER_ALLOC = 3
				total_alloc_ += per_alloc_ -1;

				for (i =1;i <per_alloc_ - 1;i++){
					item[i].id(cur_id_++);
					item[i].set_cleanup(this->cleanup,this);
					item[i].next_ = &item[i+1];
				}

				item[per_alloc_ - 1].id(cur_id_++);
				item[per_alloc_ - 1].set_cleanup(this->cleanup,this);
				item[per_alloc_ - 1].next_ = NULL;
				free_head_ = &item[2];
				free_tail_ = &item[per_alloc_ - 1];

				return &item[1];

			}else{
				return item;
			}
					
		}

		void cq_free(CQ_ITEM *item)
		{
			item->next_ = NULL;

			ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, ace_mon, this->mutex_, );
			//push  back
			if ( NULL == free_tail_ ){
				free_head_ = free_tail_ =  item;
			}else{
				free_tail_->next_ = item;
				free_tail_ = item;
			}
		}

		CQ_ITEM *	cq_pop()
		{
			cq_item_base *item;
			ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, ace_mon, this->mutex_, NULL);
			item = head_;
			if (NULL != item){
				head_ = item->next_;
				if (NULL ==head_){
					tail_ = NULL;
				}
				//bug size
				cur_size_--;
			}
			//bug size 
			//cur_size_--;
			return (CQ_ITEM*)item;
		}

		void cq_push(CQ_ITEM *item)
		{
			item->next_ = NULL;
			ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, ace_mon, this->mutex_, );
			if (NULL == tail_)
				head_ = item;
			else
				tail_->next_ = item;

			tail_ = item;
			cur_size_++;

		}

		//call by CQ_ITEM
		static void cleanup(cq_item_base* item,void* ptr)
		{
			cq_base * base =(cq_base *)ptr;
			base->cq_free((CQ_ITEM *)item);
		}

		size_t size()const
		{
			 return cur_size_;
		}

		size_t free_size()const
		{
			return total_alloc_;
		}

	protected:
		size_t cur_id_;
		size_t total_alloc_;
		size_t per_alloc_;
		size_t cur_size_;
		size_t pagesize_;

		cq_item_base * free_head_;
		cq_item_base * free_tail_;
		cq_item_base * head_;
		cq_item_base * tail_;

		CQ_ITEM * page_list_;

		ACE_SYNCH_MUTEX_T mutex_;

	};

}
#endif /* __CQ_BASE_H__ */
