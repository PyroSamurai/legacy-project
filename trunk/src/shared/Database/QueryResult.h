/*
 * Copyright (C) 2008-2008 LeGACY <http://www.legacy-project.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LEGACY_QUERYRESULT_H
#define __LEGACY_QUERYRESULT_H

class LEGACY_DLL_SPEC QueryResult
{
	public:
		QueryResult(uint64 rowCount, uint32 fieldCount)
			: mFieldCount(fieldCount), mRowCount(rowCount) {}

		virtual ~QueryResult() {}

		virtual bool NextRow() = 0;

		Field *Fetch() const { return mCurrentRow; }

		const Field & operator [] (int index) const { return mCurrentRow[index];; }

		uint32 GetFieldCount() const { return mFieldCount; }

		uint64 GetRowCount() const { return mRowCount; }

	protected:
		Field *mCurrentRow;
		uint32 mFieldCount;
		uint64 mRowCount;
};
#endif
