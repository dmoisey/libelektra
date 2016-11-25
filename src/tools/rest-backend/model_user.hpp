#ifndef ELEKTRA_REST_MODEL_USER_HEADER_GUARD
#define ELEKTRA_REST_MODEL_USER_HEADER_GUARD

#include <map>
#include <string>
#include <utility>

#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <config.hpp>
#include <exceptions.hpp>
#include <kdb_includes.hpp>

namespace kdbrest
{

namespace model
{

typedef bool (*comp_func) ();

class User : public kdb::Key
{

public:
	/**
                 * Constructs an User object based on a kdb::Key object.
                 * It is an alternative for an upwards cast.
                 * @param key A kdb::Key object
                 */
	inline User (kdb::Key & k) : kdb::Key (k)
	{
	}
	/**
                 * Constructs an User object based on a username.
                 * It implicitely constructs a kdb::Key object by super().
                 * Adds the base user repository path to the username, so it
                 * is a valid key to be use with KDB.
                 * @param username The username to be used for the key
                 */
	inline User (std::string username) : kdb::Key (ELEKTRA_REST_USER_REPOSITORY_PATH + std::string ("/") + username, KEY_END)
	{
	}

	/**
                 * Adds a key as sub key. Does check if the given key is really
                 * a sub key. If not, nothing is changed.
                 * @param key The key do be added as sub key, if it is one.
                 */
	void addSubkey (kdb::Key & k)
	{
		if (k.isBelow (static_cast<kdb::Key &> (*this)))
		{
			m_subkeys.append (k);
		}
	}
	/**
                 * Adds several keys as sub key. Relies on addSubkey(), so the
                 * conditional checks are done also for the mass add function.
                 * @param iter_start A KeySet iterator from where on sub keys should be added
				 * @param iter_end The end of the KeySet until we can read
				 * @return The iterator of the first non-child element
                 */
	kdb::KeySet::iterator addSubkeys (kdb::KeySet::iterator iter_start, kdb::KeySet::iterator iter_end)
	{
		kdb::Key & k = static_cast<kdb::Key &> (*this);
		auto elem = iter_start;
		while (elem != iter_end)
		{
			if (elem.get ().isBelow (k))
			{
				m_subkeys.append (elem.get ());
			}
			else
			{
				break; // because keyset is sorted, following keys can't be children
			}
			elem++;
		}
		return elem;
	}
	/**
                 * Getter for the kdb::KeySet containing all added sub keys.
                 * Returned will be a reference. That means changes to the
                 * subkeys keyset will affect the user. This allows for
                 * removal of subkeys.
                 * @return kdb::KeySet with all sub keys
                 */
	kdb::KeySet & getSubkeys ()
	{
		return this->m_subkeys;
	}
	/**
                 * Getter for a sub key by name. If no sub key with the
                 * given name exists, an exception will be returned.
                 * @param name The name of the sub key to look for
                 * @return The requested sub key
                 * @throws kdbrest::exception::SubkeyNotFoundException
                 */
	kdb::Key getSubkey (const std::string name) const
	{
		for (auto elem : this->m_subkeys)
		{
			if (elem.getName ().compare (this->getName () + std::string ("/") + name) == 0)
			{
				return elem;
			}
		}
		throw kdbrest::exception::SubkeyNotFoundException ();
	}

	/**
                 * Getter for the username.
                 * @return Username as string
                 */
	std::string getUsername () const
	{
		return this->getName ().erase (0, sizeof (ELEKTRA_REST_USER_REPOSITORY_PATH));
	}

	/**
                 * Setter for the password hash.
                 * @param passwordHash Password hash as string
                 */
	void setPasswordHash (std::string passwordHash)
	{
		this->set (passwordHash);
	}
	/**
                 * Getter for the password hash.
                 * @return Password hash as string
                 */
	std::string getPasswordHash () const
	{
		return this->get<std::string> ();
	}

	/**
                 * Setter for the email
                 * @param email Email as string
                 */
	void setEmail (std::string email)
	{
		kdb::Key k;
		try
		{
			k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_EMAIL);
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			k.setName (this->getName () + std::string ("/") + ELEKTRA_REST_MODEL_USER_META_EMAIL);
		}
		k.set<std::string> (email);
		this->addSubkey (k);
	}
	/**
                 * Getter for the email.
                 * @return Email as string
                 */
	std::string getEmail () const
	{
		try
		{
			kdb::Key k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_EMAIL);
			return k.get<std::string> ();
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			return "";
		}
	}

	/**
                 * Setter for the rank.
                 * 3 = Admin
                 * 2 = Moderator
                 * 1 = User
                 * @param rank Rank as integer
                 */
	void setRank (int rank)
	{
		kdb::Key k;
		try
		{
			k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_RANK);
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			k.setName (this->getName () + std::string ("/") + ELEKTRA_REST_MODEL_USER_META_RANK);
		}
		k.set<int> (rank);
		this->addSubkey (k);
	}
	/**
                 * Getter for the rank.
                 * 3 = Admin
                 * 2 = Moderator
                 * 1 = User
                 * @return Rank as integer
                 */
	int getRank () const
	{
		try
		{
			kdb::Key k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_RANK);
			return k.get<int> ();
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			return 1; // default is user rank
		}
	}

	/**
                 * Setter for the creation date.
                 * @param created_at Timestamp when the user has been created
                 */
	void setCreatedAt (const long created_at)
	{
		kdb::Key k;
		try
		{
			k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_CREATEDAT);
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			k.setName (this->getName () + std::string ("/") + ELEKTRA_REST_MODEL_USER_META_CREATEDAT);
		}
		k.set<long> (created_at);
		this->addSubkey (k);
	}
	/**
                 * Getter for the creation date.
                 * @return Timestamp when the user has been created
                 */
	long getCreatedAt () const
	{
		try
		{
			kdb::Key k = this->getSubkey (ELEKTRA_REST_MODEL_USER_META_CREATEDAT);
			return k.get<long> ();
		}
		catch (kdbrest::exception::SubkeyNotFoundException & e)
		{
			return 0;
		}
	}

	static bool less_than_username (User & l, User & r)
	{
		return boost::lexicographical_compare (l.getUsername (), r.getUsername (), boost::is_iless ());
	}

	static bool less_than_email (User & l, User & r)
	{
		return boost::lexicographical_compare (l.getEmail (), r.getEmail (), boost::is_iless ());
	}

	static bool less_than_created_at (User & l, User & r)
	{
		return l.getCreatedAt () <= r.getCreatedAt ();
	}

	static bool less_than_rank (User & l, User & r)
	{
		return l.getRank () <= r.getRank ();
	}

private:
	kdb::KeySet m_subkeys;
};

} // namespace model

} // namespace kdbrest

#endif
